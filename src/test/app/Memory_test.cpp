//------------------------------------------------------------------------------
/*
    This file is part of rippled: https://github.com/ripple/rippled
    Copyright (c) 2017 Ripple Labs Inc.
    Permission to use, copy, modify, and/or distribute this software for any
    purpose  with  or without fee is hereby granted, provided that the above
    copyright notice and this permission notice appear in all copies.
    THE  SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
    WITH  REGARD  TO  THIS  SOFTWARE  INCLUDING  ALL  IMPLIED  WARRANTIES  OF
    MERCHANTABILITY  AND  FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
    ANY  SPECIAL ,  DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
    WHATSOEVER  RESULTING  FROM  LOSS  OF USE, DATA OR PROFITS, WHETHER IN AN
    ACTION  OF  CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
    OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/
//==============================================================================

#include <ripple/app/tx/apply.h>
#include <ripple/protocol/Feature.h>
#include <string>
#include <test/jtx.h>
#include <vector>

namespace ripple {
namespace test {
struct Memory_test : public beast::unit_test::suite
{
    Json::Value
    getAcceptHook()
    {
        using namespace jtx;
        Json::Value hook;
        hook[jss::Hook] = Json::objectValue;
        hook[jss::Hook][jss::HookOn] =
            "0000000000000000000000000000000000000000000000000000000000000000";
        hook[jss::Hook][jss::HookNamespace] =
            "0000000000000000000000000000000000000000000000000000000000000000";
        hook[jss::Hook][jss::HookApiVersion] = 0;
        hook[jss::Hook][jss::Flags] = 1;
        hook[jss::Hook][jss::CreateCode] = strHex(XahauGenesis::AcceptHook);
        return hook;
    }

    Json::Value
    setAcceptHook(jtx::Account const& account)
    {
        using namespace jtx;
        Json::Value tx;
        tx[jss::Account] = account.human();
        tx[jss::TransactionType] = "SetHook";
        tx[jss::Hooks] = Json::arrayValue;
        tx[jss::Hooks][0u] = Json::objectValue;
        tx[jss::Hooks][0u][jss::Hook] = Json::objectValue;
        tx[jss::Hooks][0u][jss::Hook][jss::HookOn] =
            "0000000000000000000000000000000000000000000000000000000000000000";
        tx[jss::Hooks][0u][jss::Hook][jss::HookNamespace] =
            "0000000000000000000000000000000000000000000000000000000000000000";
        tx[jss::Hooks][0u][jss::Hook][jss::HookApiVersion] = 0;
        tx[jss::Hooks][0u][jss::Hook][jss::Flags] = 5;
        tx[jss::Hooks][0u][jss::Hook][jss::CreateCode] = strHex(XahauGenesis::AcceptHook);
        return tx;
    }

    std::unique_ptr<Config>
    makeNetworkConfig(uint32_t networkID)
    {
        using namespace jtx;
        return envconfig([&](std::unique_ptr<Config> cfg) {
            cfg->NETWORK_ID = networkID;
            Section config;
            config.append(
                {"reference_fee = 10",
                 "account_reserve = 1000000",
                 "owner_reserve = 200000"});
            auto setup = setup_FeeVote(config);
            cfg->FEES = setup;
            return cfg;
        });
    }

    void
    profilePaymentNoHook(FeatureBitset features)
    {
        testcase("no hook tx");

        using namespace test::jtx;
        using namespace std::literals;
        {
            test::jtx::Env env{*this, makeNetworkConfig(21337)};

            auto const alice = Account("alice");
            auto const issuer = Account("issuer");
            env.fund(XRP(10000), alice, issuer);
            env.close();

            env(pay(alice, issuer, XRP(10)), fee(XRP(2)));
            env.close();
        }
    }

    void
    profilePaymentOneHook(FeatureBitset features)
    {
        testcase("one hook tx");

        using namespace test::jtx;
        using namespace std::literals;
        {
            test::jtx::Env env{*this, makeNetworkConfig(21337)};

            auto const alice = Account("alice");
            auto const issuer = Account("issuer");
            env.fund(XRP(10000), alice, issuer);
            env.close();

            Json::Value tx;
            tx[jss::Account] = alice.human();
            tx[jss::TransactionType] = "SetHook";
            tx[jss::Hooks] = Json::arrayValue;
            for(int i = 0; i < 1; i++) {
                tx[jss::Hooks][i] = getAcceptHook();
            }

            env(tx, fee(XRP(10)));

            env(pay(alice, issuer, XRP(10)), fee(XRP(2)));
            env.close();
        }
    }

    void
    profilePaymentMaxHook(FeatureBitset features)
    {
        testcase("max hook tx");

        using namespace test::jtx;
        using namespace std::literals;
        {
            test::jtx::Env env{*this, makeNetworkConfig(21337)};

            auto const alice = Account("alice");
            auto const issuer = Account("issuer");
            env.fund(XRP(10000), alice, issuer);
            env.close();

            Json::Value tx;
            tx[jss::Account] = alice.human();
            tx[jss::TransactionType] = "SetHook";
            tx[jss::Hooks] = Json::arrayValue;
            for(int i = 0; i < 10; i++) {
                tx[jss::Hooks][i] = getAcceptHook();
            }

            env(tx, fee(XRP(10)));

            env(pay(alice, issuer, XRP(10)), fee(XRP(2)));
            env.close();
        }
    }

    void
    profilePaymentMaxHookBoth(FeatureBitset features)
    {
        testcase("max hook tx both");

        using namespace test::jtx;
        using namespace std::literals;
        {
            test::jtx::Env env{
                *this,
                makeNetworkConfig(21337),
                nullptr,
                beast::severities::kTrace};

            auto const alice = Account("alice");
            auto const issuer = Account("issuer");
            env.fund(XRP(10000), alice, issuer);
            env.close();

            Json::Value tx1;
            tx1[jss::Account] = alice.human();
            tx1[jss::TransactionType] = "SetHook";
            tx1[jss::Hooks] = Json::arrayValue;
            for(int i = 0; i < 10; i++) {
                tx1[jss::Hooks][i] = getAcceptHook();
            }
            env(tx1, fee(XRP(10)));

            Json::Value tx2;
            tx2[jss::Account] = issuer.human();
            tx2[jss::TransactionType] = "SetHook";
            tx2[jss::Hooks] = Json::arrayValue;
            for(int i = 0; i < 10; i++) {
                tx2[jss::Hooks][i] = getAcceptHook();
            }
            env(tx2, fee(XRP(10)));

            env(pay(alice, issuer, XRP(10)), fee(XRP(2)));
            env.close();
        }
    }

    void
    testHookTest(FeatureBitset features)
    {
        testcase("hook tx");

        using namespace test::jtx;
        using namespace std::literals;

        {
            // test::jtx::Env env{*this, makeNetworkConfig(21337)};
            test::jtx::Env env{
                *this,
                makeNetworkConfig(21337),
                nullptr,
                beast::severities::kTrace};

            auto const alice = Account("alice");
            auto const issuer = Account("issuer");
            env.fund(XRP(10000), alice, issuer);
            env.close();

            std::string const createCodeHex =
                "0061736D0100000001350860057F7F7F7F7F017E60027F7F017E60037F7F7F017E60017F017E6000017E60047F7F7F7F017E60037F7F7E017E60027F7F017F029D010A03656E76057472616365000003656E760C686F6F6B5F6163636F756E74000103656E760A6F74786E5F6669656C64000203656E760C6574786E5F72657365727665000303656E760A6C65646765725F736571000403656E760C6574786E5F64657461696C73000103656E760D6574786E5F6665655F62617365000103656E7604656D6974000503656E7606616363657074000603656E76025F6700070302010305030100020631087F0141B08B040B7F004190090B7F004180080B7F0041AB0B0B7F004180080B7F0041B08B040B7F0041000B7F0041010B07080104686F6F6B000A0AA8880001A4880001027F23004190016B220124002001200036028C0141F008411D41D408411C410010001A418D0A411410011A2001200141E8006A4108418180181002370360200120012D0068410776047E427E05200131006F20013100684291A2C480B0018342388620013100694230867C200131006A4228867C200131006B4220867C200131006C4218867C200131006D4210867C200131006E4208867C7C0B37035820012903604208510440200141AF093602542001290358423887423F8342407DA7210020012001280254220241016A360254200220003A0000200129035842308742FF0183A7210020012001280254220241016A360254200220003A0000200129035842288742FF0183A7210020012001280254220241016A360254200220003A0000200129035842208742FF0183A7210020012001280254220241016A360254200220003A0000200129035842188742FF0183A7210020012001280254220241016A360254200220003A0000200129035842108742FF0183A7210020012001280254220241016A360254200220003A0000200129035842088742FF0183A7210020012001280254220241016A360254200220003A0000200129035842FF0183A7210020012001280254220241016A360254200220003A00000B41A30A41144181802010021A410110031A20011004A741016A36025041A4092001280250220041FF017141187420004180FE0371410874722000418080FC0771410876722000418080807871411876723602002001200128025041046A36024C41AA09200128024C220041FF017141187420004180FE0371410874722000418080FC077141087672200041808080787141187672360200419E0941044183800810024204510440419D09412E3A00000B41B70A41F40010051A2001419009419B021006370340200141E00936023C2001290340423887423F8342407DA721002001200128023C220241016A36023C200220003A0000200129034042308742FF0183A721002001200128023C220241016A36023C200220003A0000200129034042288742FF0183A721002001200128023C220241016A36023C200220003A0000200129034042208742FF0183A721002001200128023C220241016A36023C200220003A0000200129034042188742FF0183A721002001200128023C220241016A36023C200220003A0000200129034042108742FF0183A721002001200128023C220241016A36023C200220003A0000200129034042088742FF0183A721002001200128023C220241016A36023C200220003A0000200129034042FF0183A721002001200128023C220241016A36023C200220003A00004180084103419009419B02410110001A2001200141106A4120419009419B02100737030820012903084200550440418408412842F00010081A0B41AC08412842F20010081A4101410110091A20014190016A240042000B0B850204004180080B8D0174786E0074786E5F7061796D656E745F6C6F6F702E633A20547820656D697474656420737563636573732E0074786E5F7061796D656E745F6C6F6F702E633A20547820656D6974746564206661696C7572652E0074786E5F7061796D656E745F6C6F6F702E633A2043616C6C65642E002274786E5F7061796D656E745F6C6F6F702E633A2043616C6C65642E22004190090B5A120000228000000024000000009999999999201A00000000201B0000000061999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999684000000000000000732100418B0A0B0281140041A10A0B028314";
            std::string ns_str =
                "CAFECAFECAFECAFECAFECAFECAFECAFECAFECAFECAFECAFECAFECAFECAFECA"
                "FE";
            Json::Value jv =
                ripple::test::jtx::hook(issuer, {{hso(createCodeHex)}}, 0);
            jv[jss::Hooks][0U][jss::Hook][jss::HookNamespace] = ns_str;
            jv[jss::Hooks][0U][jss::Hook][jss::HookOn] =
                "0000000000000000000000000000000000000000000000000000000000000000";
            env(jv, fee(XRP(2)));
            env.close();
            Json::Value jv1 =
                ripple::test::jtx::hook(alice, {{hso(createCodeHex)}}, 0);
            jv1[jss::Hooks][0U][jss::Hook][jss::HookNamespace] = ns_str;
            jv1[jss::Hooks][0U][jss::Hook][jss::HookOn] =
                "0000000000000000000000000000000000000000000000000000000000000000";
            env(jv1, fee(XRP(2)));
            env.close();

            env(pay(alice, issuer, XRP(10)), fee(XRP(2)));
            env.close();

            std::vector<std::shared_ptr<STTx const>> txns;
            for (int i = 0; i < 10; ++i)
            {
                if(auto tx = env.tx())
                {
                    std::cout << "close(): " << i << "\n";
                    txns.emplace_back(tx);
                    env.close();
                }
            }
            std::cout << "txns.size(): " << txns.size() << "\n";
            BEAST_EXPECT(txns.size() == 4);
            // const char* COMMAND = jss::tx.c_str();
            // for (size_t i = 0; i < txns.size(); ++i)
            // {
            //     auto const& tx = txns[i];
            //     std::cout << "tx: " << to_string(tx->getTransactionID()) << "\n";
            //     auto const result = env.rpc(COMMAND, to_string(tx->getTransactionID()));
            //     auto const meta = jrr[jss::result][jss::meta];
            //     auto const emissions = meta[sfHookEmissions.jsonName];
            //     auto const emission = emissions[0u][sfHookEmission.jsonName];
            //     auto const txId = emission[sfEmittedTxnID.jsonName];
            //     std::cout << "result: " << result << "\n";
            // }
        }
    }

    void
    testProfiling(FeatureBitset features)
    {
        profilePaymentNoHook(features);
        profilePaymentOneHook(features);
        profilePaymentMaxHook(features);
        profilePaymentMaxHookBoth(features);
    }

    void
    testWithFeats(FeatureBitset features)
    {
        testHookTest(features);
    }

public:
    void
    run() override
    {
        using namespace test::jtx;
        auto const sa = supported_amendments();
        testWithFeats(sa);
        // testProfiling(sa);
    }
};

BEAST_DEFINE_TESTSUITE(Memory, app, ripple);

}  // namespace test
}  // namespace ripple
