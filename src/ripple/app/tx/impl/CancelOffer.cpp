//------------------------------------------------------------------------------
/*
    This file is part of rippled: https://github.com/ripple/rippled
    Copyright (c) 2012, 2013 Ripple Labs Inc.

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

#include <ripple/app/tx/impl/CancelOffer.h>
#include <ripple/basics/Log.h>
#include <ripple/ledger/View.h>
#include <ripple/protocol/st.h>

namespace ripple {

NotTEC
CancelOffer::preflight(PreflightContext const& ctx)
{
    if (auto const ret = preflight1(ctx); !isTesSuccess(ret))
        return ret;

    auto const uTxFlags = ctx.tx.getFlags();

    if (uTxFlags & tfUniversalMask)
    {
        JLOG(ctx.j.trace()) << "Malformed transaction: "
                            << "Invalid flags set.";
        return temINVALID_FLAG;
    }

    if (!ctx.tx[sfOfferSequence])
    {
        JLOG(ctx.j.trace()) << "CancelOffer::preflight: missing sequence";
        return temBAD_SEQUENCE;
    }

    return preflight2(ctx);
}

//------------------------------------------------------------------------------

TER
CancelOffer::preclaim(PreclaimContext const& ctx)
{
    auto const id = ctx.tx[sfAccount];
    auto const offerSequence = ctx.tx[sfOfferSequence];

    auto const sle = ctx.view.read(keylet::account(id));
    if (!sle)
        return terNO_ACCOUNT;

    bool hooksEnabled = ctx.view.rules().enabled(featureHooks);

    auto const offerID = ctx.tx[~sfOfferID];

    if ((*sle)[sfSequence] <= offerSequence)
    {
        if (hooksEnabled && offerID && offerSequence == 0)
        {
            // pass, here the txn provides offerID instead of offerSequence
        }
        else
        {
            JLOG(ctx.j.trace())
                << "Malformed transaction: "
                << "Sequence " << offerSequence << " is invalid.";
            return temBAD_SEQUENCE;
        }
    }

    return tesSUCCESS;
}

//------------------------------------------------------------------------------

TER
CancelOffer::doApply()
{
    auto const offerSequence = ctx_.tx[sfOfferSequence];

    auto const sle = view().read(keylet::account(account_));
    if (!sle)
        return tefINTERNAL;

    bool hooksEnabled = view().rules().enabled(featureHooks);

    std::optional<uint256> offerID;

    if (hooksEnabled)
        offerID = ctx_.tx[~sfOfferID];

    Keylet cancel = hooksEnabled && offerID && offerSequence == 0
        ? Keylet(ltOFFER, *offerID)
        : keylet::offer(account_, offerSequence);

    if (auto sleOffer = view().peek(cancel))
    {
        if (offerID)
            JLOG(j_.debug()) << "Trying to cancel offer :" << *offerID;
        else
            JLOG(j_.debug()) << "Trying to cancel offer #" << offerSequence;
        return offerDelete(view(), sleOffer, ctx_.app.journal("View"));
    }

    if (offerID)
        JLOG(j_.debug()) << "Offer :" << *offerID << " can't be found.";
    else
        JLOG(j_.debug()) << "Offer #" << offerSequence << " can't be found.";
    return tesSUCCESS;
}

}  // namespace ripple
