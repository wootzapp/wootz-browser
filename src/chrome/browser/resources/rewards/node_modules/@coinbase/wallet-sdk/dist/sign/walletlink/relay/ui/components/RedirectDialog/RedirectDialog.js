"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
exports.RedirectDialog = void 0;
const clsx_1 = __importDefault(require("clsx"));
const preact_1 = require("preact");
const cssReset_1 = require("../cssReset/cssReset");
const Snackbar_1 = require("../Snackbar/Snackbar");
const util_1 = require("../util");
const RedirectDialog_css_1 = __importDefault(require("./RedirectDialog-css"));
class RedirectDialog {
    constructor() {
        this.root = null;
        this.darkMode = (0, util_1.isDarkMode)();
    }
    attach() {
        const el = document.documentElement;
        this.root = document.createElement('div');
        this.root.className = '-cbwsdk-css-reset';
        el.appendChild(this.root);
        (0, cssReset_1.injectCssReset)();
    }
    present(props) {
        this.render(props);
    }
    clear() {
        this.render(null);
    }
    render(props) {
        if (!this.root)
            return;
        (0, preact_1.render)(null, this.root);
        if (!props)
            return;
        (0, preact_1.render)((0, preact_1.h)(RedirectDialogContent, Object.assign({}, props, { onDismiss: () => {
                this.clear();
            }, darkMode: this.darkMode })), this.root);
    }
}
exports.RedirectDialog = RedirectDialog;
const RedirectDialogContent = ({ title, buttonText, darkMode, onButtonClick, onDismiss }) => {
    const theme = darkMode ? 'dark' : 'light';
    return ((0, preact_1.h)(Snackbar_1.SnackbarContainer, { darkMode: darkMode },
        (0, preact_1.h)("div", { class: "-cbwsdk-redirect-dialog" },
            (0, preact_1.h)("style", null, RedirectDialog_css_1.default),
            (0, preact_1.h)("div", { class: "-cbwsdk-redirect-dialog-backdrop", onClick: onDismiss }),
            (0, preact_1.h)("div", { class: (0, clsx_1.default)('-cbwsdk-redirect-dialog-box', theme) },
                (0, preact_1.h)("p", null, title),
                (0, preact_1.h)("button", { onClick: onButtonClick }, buttonText)))));
};
