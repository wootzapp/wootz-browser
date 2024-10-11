import { useMutation } from "@tanstack/react-query";
import { waitForReceipt, } from "../../../../transaction/actions/wait-for-tx-receipt.js";
import { stringify } from "../../../../utils/json.js";
export const useTransactionButtonMutation = (props, sendTransactionFn) => {
    const { transaction, onTransactionSent, onTransactionConfirmed, onError, onClick, } = props;
    return useMutation({
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
                    const receipt = await waitForReceipt(result);
                    if (receipt.status === "reverted") {
                        throw new Error(`Execution reverted: ${stringify(receipt, null, 2)}`);
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
//# sourceMappingURL=transaction-button-utils.js.map