"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.fadeInAnimation = exports.floatDownAnimation = exports.floatUpAnimation = void 0;
const react_1 = require("@emotion/react");
exports.floatUpAnimation = (0, react_1.keyframes) `
  from {
    opacity: 0;
    transform: translateY(20%) scale(0.8) ;
  }
  to {
    opacity: 1;
    transform: translateY(0);
  }
`;
exports.floatDownAnimation = (0, react_1.keyframes) `
  from {
    opacity: 0;
    transform: translateY(-20%) scale(0.8) ;
  }
  to {
    opacity: 1;
    transform: translateY(0);
  }
`;
exports.fadeInAnimation = (0, react_1.keyframes) `
  from {
    opacity: 0;
  }
  to {
    opacity: 1;
  }
`;
//# sourceMappingURL=animations.js.map