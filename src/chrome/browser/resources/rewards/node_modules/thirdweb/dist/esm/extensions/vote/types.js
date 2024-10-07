export var VoteType;
(function (VoteType) {
    VoteType[VoteType["against"] = 0] = "against";
    VoteType[VoteType["for"] = 1] = "for";
    VoteType[VoteType["abstain"] = 2] = "abstain";
})(VoteType || (VoteType = {}));
export var ProposalState;
(function (ProposalState) {
    ProposalState[ProposalState["pending"] = 0] = "pending";
    ProposalState[ProposalState["active"] = 1] = "active";
    ProposalState[ProposalState["canceled"] = 2] = "canceled";
    ProposalState[ProposalState["defeated"] = 3] = "defeated";
    ProposalState[ProposalState["succeeded"] = 4] = "succeeded";
    ProposalState[ProposalState["queued"] = 5] = "queued";
    ProposalState[ProposalState["expired"] = 6] = "expired";
    ProposalState[ProposalState["executed"] = 7] = "executed";
})(ProposalState || (ProposalState = {}));
//# sourceMappingURL=types.js.map