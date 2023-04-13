const version = '1.0';
const rac = require('ripple-address-codec');
const fs = require('fs');
const os = require("os");

const print_args = (err)=>
{
    if (err)
    {
        console.log("Error: " + err);
        console.log("");
    }
    console.log(
`Xahau Governance CLI
Version: ` + version + `
Usage: node govcli [-f seedfile] wss://node goverance-r-address topic topic-data
Reads from ~/.xahau_governance_seed by default. Use -f to change seed file location.
Topics:
    Topic           |   Data Type
    ----------------------------------------------------------------------------
    REWARD_RATE     | Claim period interest percentage e.g. "0.04" (means 0.04%)
    REWARD_DELAY    | Claim period (typically a month) in seconds e.g. 2592000
    HOOK_0          | The 32 byte HookHash to install at Hook position 0 on the 
    ..              | genesis account
    HOOK_9  
    SEAT_0          | The r-address to install at seat zero of the governance
    ..              | table.
    SEAT_19 
    BATCH_0         | The amount in drops to reimburse the member at seat 0
    ..              | for xrp swaps that the other members deem have occured
    BATCH_19
To delete or unset a previous vote type DELETE instead of the topic-data.
`
    );
    process.exit(1);
}

let argv = process.argv;
let upto = 2;

if (argv.length - upto < 3)
    return print_args();

const seedfile = argv[upto] == '-f' ? argv[upto+1] : os.homedir() + '/.xahau_governance_seed';

if (argv[upto] == '-f')
    upto += 2;

if (argv.length - upto < 4)
    return print_args();

const uri = argv[upto++];
const address = argv[upto++];
const topic = argv[upto++].toUpperCase();
const topic_data_raw = argv[upto ++];

const is_delete = (topic_data_raw == 'DELETE');

const valid_topics = (()=>
{
    let valid_topics = {
        REWARD_RATE: 1,
        REWARD_DELAY: 1
    };

    for (let i = 0; i < 20; ++i)
    {
        valid_topics['SEAT_' + i] = '530' + ( i < 10 ? '0' + i : i);
        valid_topics['BATCH_' + i] = '42' + ( i < 10 ? '0' + i : i);
        if (i > 9)
            continue;
        valid_topics['HOOK_' + i] = '480' + i;
    }
    return valid_topics;
})();

if (valid_topics[topic] === undefined)
    return print_args("Invalid topic: `" + topic + "`");

let topic_parts = topic.split("_");
const topic_type = topic_parts[0];
const topic_field = topic_parts[1];

require('./utils-tests.js').TestRig(uri).then(t=>
{

    let topic_data;

    // topic validation
    switch (topic_type)
    {
        case 'REWARD':
        {
            if (is_delete)
                return print_args("Cannot delete votes for REWARD topics");
        }
        case 'BATCH':
        {
            if (is_delete)
            {
                topic_data = "0000000000000000";
                break;
            }
    
            try
            {
                if (topic_field == 'RATE')
                {
                    if (topic_data_raw.match(/^[0-9]+(\.[0-9]+)?$/) === null)
                        return print_args(
                            "REWARD_RATE must be a percentage from 0 to 100. e.g. 0.04 (meaning 0.04%)");

                    let parts = topic_data_raw.split('.')
                    
                    let exp = 0;
                    let mantissa = parts[0] + '';
                    if (parts[1] !== undefined && parts[1].length > 0)
                    {
                        exp -= parts[1].length;
                        mantissa += parts[1];
                    }

                    topic_data = t.xfl(exp, mantissa);
                    if (t.is_negative(topic_data))
                        return print_args("REWARD_RATE cannot be negative.");
                } else
                    topic_data = parseInt('' + topic_data_raw);

                if (topic_data <= 0)
                    return print_args(topic_type + " topics must be > 0.");

                topic_data = topic_data.
                    toString(16).
                    toUpperCase();

                if (topic_data.length < 16)
                    topic_data = '0'.repeat(16 - topic_data.length) + topic_data;
                
                topic_data = topic_data.
                    match(/.{1,2}/g).
                    reverse().
                    join(''); // endian flip

            } catch (e)
            {
                return print_args(
                    "REWARD topic data invalid. Could not parse provided argument. " + e);
            }
            break;
        }

        case 'SEAT':
        {
            try
            {
                topic_data = 
                    is_delete ? '0'.repeat(40) :
                        rac.decodeAccountID(topic_data_raw).toString('hex').toUpperCase();
            } catch (e)
            {
                return print_args("SEAT topic data must be an r-address. Could not parse provided argument.");
            }
            break;
        }

        case 'HOOK':
        {
            topic_data =
                is_delete ? '0'.repeat(64) : topic_data_raw.toUpperCase();

            if (topic_data.match(/^[0-9A-F]{64}$/) === null)
                return print_args(
                    "HOOK topic data must be a 32 byte HookHash presented as 64 hex nibbles." +
                    "Could not parse provided argument");
            break;
        }
        default:
            return print_args();
    }

    console.log("Topic: " + topic_type + "_" + topic_field + ", Data: " + topic_data);
    
    // submit txn

    const secret = (()=>
    {
        try
        {
            return fs.readFileSync(seedfile).toString('utf-8');
        } catch(e)
        {
            return print_args("Could not read seedfile: " + seedfile + ' ' + e);
        }
    })();
    
    const txn =
    {
        TransactionType: "Invoke",
        Account: address,
        HookParameters:
        [
            {
                HookParameter:
                {
                    HookParameterName: "54", // T = topic 
                    HookParameterValue: valid_topics[topic]
                }
            },
            {
                HookParameter:
                {
                    HookParameterName: "56", // V = vote
                    HookParameterValue: topic_data
                }
            }
        ]
    };

    console.log("Submitting...", txn);

    t.feeSubmit(secret,txn).then(x=>
    {
        console.log(x);
        process.exit(0);
    }).catch(t.err);


}).catch(e => {
    console.log("Could not connect to " + uri);
    process.exit(1);
});
