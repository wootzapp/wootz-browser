import { ONE_THOUSAND } from "../constants";
export function toMiliseconds(seconds) {
    return seconds * ONE_THOUSAND;
}
export function fromMiliseconds(miliseconds) {
    return Math.floor(miliseconds / ONE_THOUSAND);
}
//# sourceMappingURL=convert.js.map