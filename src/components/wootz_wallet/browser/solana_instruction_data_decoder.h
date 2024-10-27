/* Copyright (c) 2022 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef COMPONENTS_WOOTZ_WALLET_BROWSER_SOLANA_INSTRUCTION_DATA_DECODER_H_
#define COMPONENTS_WOOTZ_WALLET_BROWSER_SOLANA_INSTRUCTION_DATA_DECODER_H_

#include <optional>
#include <string>
#include <vector>

#include "components/wootz_wallet/browser/solana_instruction_decoded_data.h"
#include "components/wootz_wallet/common/wootz_wallet.mojom.h"

namespace wootz_wallet::solana_ins_data_decoder {

std::optional<SolanaInstructionDecodedData> Decode(
    base::span<const uint8_t> data,
    const std::string& program_id);

std::optional<uint8_t> DecodeUint8(base::span<const uint8_t> input,
                                   size_t& offset);

std::optional<uint32_t> DecodeUint32(base::span<const uint8_t> input,
                                     size_t& offset);

std::optional<std::string> DecodePublicKey(base::span<const uint8_t> input,
                                           size_t& offset);

std::optional<mojom::SolanaSystemInstruction> GetSystemInstructionType(
    base::span<const uint8_t> data,
    const std::string& program_id);

std::optional<mojom::SolanaComputeBudgetInstruction>
GetComputeBudgetInstructionType(const std::vector<uint8_t>& data,
                                const std::string& program_id);

bool IsCompressedNftTransferInstruction(const std::vector<uint8_t>& data,
                                        const std::string& program_id);

std::vector<InsParamPair> GetAccountParamsForTesting(
    std::optional<mojom::SolanaSystemInstruction> sys_ins_type,
    std::optional<mojom::SolanaTokenInstruction> token_ins_type);

std::vector<mojom::SolanaInstructionAccountParamPtr>
GetMojomAccountParamsForTesting(
    std::optional<mojom::SolanaSystemInstruction> sys_ins_type,
    std::optional<mojom::SolanaTokenInstruction> token_ins_type);

}  // namespace wootz_wallet::solana_ins_data_decoder

#endif  // COMPONENTS_WOOTZ_WALLET_BROWSER_SOLANA_INSTRUCTION_DATA_DECODER_H_
