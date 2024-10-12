"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.verifyJWT = exports.signJWT = exports.generateKeyPair = void 0;
const tslib_1 = require("tslib");
const ed25519 = tslib_1.__importStar(require("@stablelib/ed25519"));
const random_1 = require("@stablelib/random");
const time_1 = require("@walletconnect/time");
const constants_1 = require("./constants");
const utils_1 = require("./utils");
function generateKeyPair(seed = random_1.randomBytes(constants_1.KEY_PAIR_SEED_LENGTH)) {
    return ed25519.generateKeyPairFromSeed(seed);
}
exports.generateKeyPair = generateKeyPair;
function signJWT(sub, aud, ttl, keyPair, iat = time_1.fromMiliseconds(Date.now())) {
    return tslib_1.__awaiter(this, void 0, void 0, function* () {
        const header = { alg: constants_1.JWT_IRIDIUM_ALG, typ: constants_1.JWT_IRIDIUM_TYP };
        const iss = utils_1.encodeIss(keyPair.publicKey);
        const exp = iat + ttl;
        const payload = { iss, sub, aud, iat, exp };
        const data = utils_1.encodeData({ header, payload });
        const signature = ed25519.sign(keyPair.secretKey, data);
        return utils_1.encodeJWT({ header, payload, signature });
    });
}
exports.signJWT = signJWT;
function verifyJWT(jwt) {
    return tslib_1.__awaiter(this, void 0, void 0, function* () {
        const { header, payload, data, signature } = utils_1.decodeJWT(jwt);
        if (header.alg !== constants_1.JWT_IRIDIUM_ALG || header.typ !== constants_1.JWT_IRIDIUM_TYP) {
            throw new Error("JWT must use EdDSA algorithm");
        }
        const publicKey = utils_1.decodeIss(payload.iss);
        return ed25519.verify(publicKey, data, signature);
    });
}
exports.verifyJWT = verifyJWT;
//# sourceMappingURL=api.js.map