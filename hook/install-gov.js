require('./utils-tests.js').TestRig('ws://localhost:6006').then(t=>
{
    t.feeSubmit(t.genesis.seed,
    {
        Account: t.genesis.classicAddress,
        TransactionType: "SetHook",
        NetworkID: 21339,
        Hooks:
        [
            {
                Hook:
                {
                    Flags: 1,
                    CreateCode: t.wasm("reward.wasm"), 
                    HookApiVersion: 0,
                    HookNamespace: "0000000000000000000000000000000000000000000000000000000000000000",
                    HookOn: "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFBFFFFFFFFFFFFFFFFFFBFFFFF"
                }
            },
            {
                Hook:
                {
                    Flags: 1,
                    CreateCode: t.wasm("govern.wasm"), 
                    HookApiVersion: 0,
                    HookNamespace: "0000000000000000000000000000000000000000000000000000000000000000",
                    HookOn: "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF7FFFFFFFFFFFFFFFFFFBFFFFF",
                    HookParameters:
                    [
                        {
                            HookParameter:
                            {
                                HookParameterName:  "494D43",   // IMC
                                HookParameterValue: "0500000000000000"
                            }
                        },
                        {
                            HookParameter:
                            {
                                HookParameterName:  "494441",   // IDA
                                HookParameterValue: "00A0724E18090000"
                            }
                        },
                        {
                            HookParameter:
                            {
                                HookParameterName:  "495252",    // IRR
                                HookParameterValue: "A03FDFDE78908354"  
                            }
                        },
                        {
                            HookParameter:
                            {
                                HookParameterName:  "495244",    // IRD
                                HookParameterValue: "3C00000000000000"
                            }
                        },
                        {
                            HookParameter:
                            {
                                /*
                                 * r4iASZ9imoYpcJ19coK6gaun4y7iqSc1G2
                                 * snCdAifi2puDdUXcAsZs7HyoiqY3e
                                 */
                                HookParameterName:  "495300",    // IS\x00
                                HookParameterValue: "EFCC77E2D504CAD0C9E07D5FD9C7CBBBC5CF0FAB"
                            }
                        },
                        {
                            HookParameter:
                            {
                                /*
                                 * rQKoKmjnmzTJbNQopwsxktFAaVgAnY9SaP
                                 * shPu5k91unMpgjokit7ixcbFAshhB
                                 */
                                HookParameterName:  "495301",    // IS\x01
                                HookParameterValue: "FFD90CA3495626D6005367B4E8CCE05393D5EC44"
                            }
                        },
                        {
                            HookParameter:
                            {
                                /*
                                 * r4U1A4QQQU1uNfAaDcfWjhJM1M74eJdw4A
                                 * ssDCiXDvE6B7qiGbLTTVMCZwe4QNC
                                 */
                                HookParameterName:  "495302",    // IS\x02
                                HookParameterValue: "E899BCC8A70F0D124F1B7F7A2FE5D915D85C0C07"
                            }
                        },
                        {
                            HookParameter:
                            {
                                /*
                                 * rwwWHqB1r5wsrHeCHE5BX9cCwQgTjvmVpS
                                 * sh7CTQMEke8eguwbTg15JQT28xgPM
                                 */
                                HookParameterName:  "495303",    // IS\x03
                                HookParameterValue: "6485536FEAA6579250E013BC8C77BE62E98CDD6F"
                            }
                        },
                        {
                            HookParameter:
                            {
                                /*
                                 * rPMXfrFDREUggGhNhPtzE2iPRFYVp6Jk1m
                                 * sn48jTu6oqyAgAjAxEsXKMQq6CQx8
                                 */
                                HookParameterName:  "495304",    // IS\x04
                                HookParameterValue: "F534373CF7575B900E6935DC18ACEE5BB3D8065F"
                            }
                        }
                    ]
                }
            }
        ]
    }).then(x=>
    {
        t.assertTxnSuccess(x)
        console.log(x);
        process.exit(0);
    }).catch(t.err);
})



