import { NumberNode } from './parsers';
/**
 * Ensures that a given number is expressed in radians. If the number is already
 * in radians, does nothing. If the value is in degrees, converts it to radians.
 * If the value has no specified unit, the unit is assumed to be radians. If the
 * value is not in radians or degrees, the value is resolved as 0 radians.
 *
 * Also accepts a second argument that is a default value to use if the input
 * numberNode number is NaN or Infinity.
 */
export declare const degreesToRadians: (numberNode: NumberNode, fallbackRadianValue?: number) => NumberNode;
/**
 * Ensures that a given number is expressed in degrees. If the number is already
 * in degrees, does nothing. If the value is in radians or has no specified
 * unit, converts it to degrees. If the value is not in radians or degrees, the
 * value is resolved as 0 degrees.
 *
 * Also accepts a second argument that is a default value to use if the input
 * numberNode number is NaN or Infinity.
 */
export declare const radiansToDegrees: (numberNode: NumberNode, fallbackDegreeValue?: number) => NumberNode;
/**
 * Converts a given length to meters. Currently supported input units are
 * meters, centimeters and millimeters.
 *
 * Also accepts a second argument that is a default value to use if the input
 * numberNode number is NaN or Infinity.
 */
export declare const lengthToBaseMeters: (numberNode: NumberNode, fallbackMeterValue?: number) => NumberNode;
/**
 * Normalizes the unit of a given input number so that it is expressed in a
 * preferred unit. For length nodes, the return value will be expressed in
 * meters. For angle nodes, the return value will be expressed in radians.
 *
 * Also takes a fallback number that is used when the number value is not a
 * valid number or when the unit of the given number cannot be normalized.
 */
export declare const normalizeUnit: (node: NumberNode, fallback?: NumberNode) => NumberNode<import("./parsers").Unit | "%" | null>;
