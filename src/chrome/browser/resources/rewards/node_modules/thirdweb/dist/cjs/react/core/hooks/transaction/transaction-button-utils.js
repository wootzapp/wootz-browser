"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.useTransactionButtonMutation = void 0;
const react_query_1 = require("@tanstack/react-query");
const wait_for_tx_receipt_js_1 = require("../../../../transaction/actions/wait-for-tx-receipt.js");
const json_js_1 = require("../../../../utils/json.js");
const useTransactionButtonMutation = (props, sendTransactionFn) => {
    const { transaction, onTransactionSent, onTransactionConfirmed, onError, onClick, } = props;
    return (0, react_query_1.useMutation)({
        mutationFn: async () => {
            if (onClick) {
                onClick();
            }
            try {
                const resolvedTx = await transaction();
                const result = await sendTransactionFn(resolvedTx);
                if (onTransactionSent) {
                    onTransactionSent(result);
                }
                if (onTransactionConfirmed) {
                    const receipt = await (0, wait_for_tx_receipt_js_1.waitForReceipt)(result);
                    if (receipt.status === "reverted") {
                        throw new Error(`Execution reverted: ${(0, json_js_1.stringify)(receipt, null, 2)}`);
                    }
                    onTransactionConfirmed(receipt);
                }
            }
            catch (error) {
                if (onError) {
                    onError(error);
                }
            }
            finally {
            }
        },
    });
};
exports.useTransactionButtonMutation = useTransactionButtonMutation;
//# sourceMappingURL=transaction-button-utils.js.map