/**
 * Reads 2 bytes from array starting at offset as big-endian
 * signed 16-bit integer and returns it.
 */
export declare function readInt16BE(array: Uint8Array, offset?: number): number;
/**
 * Reads 2 bytes from array starting at offset as big-endian
 * unsigned 16-bit integer and returns it.
 */
export declare function readUint16BE(array: Uint8Array, offset?: number): number;
/**
 * Reads 2 bytes from array starting at offset as little-endian
 * signed 16-bit integer and returns it.
 */
export declare function readInt16LE(array: Uint8Array, offset?: number): number;
/**
 * Reads 2 bytes from array starting at offset as little-endian
 * unsigned 16-bit integer and returns it.
 */
export declare function readUint16LE(array: Uint8Array, offset?: number): number;
/**
 * Writes 2-byte big-endian representation of 16-bit unsigned
 * value to byte array starting at offset.
 *
 * If byte array is not given, creates a new 2-byte one.
 *
 * Returns the output byte array.
 */
export declare function writeUint16BE(value: number, out?: Uint8Array, offset?: number): Uint8Array;
export declare const writeInt16BE: typeof writeUint16BE;
/**
 * Writes 2-byte little-endian representation of 16-bit unsigned
 * value to array starting at offset.
 *
 * If byte array is not given, creates a new 2-byte one.
 *
 * Returns the output byte array.
 */
export declare function writeUint16LE(value: number, out?: Uint8Array, offset?: number): Uint8Array;
export declare const writeInt16LE: typeof writeUint16LE;
/**
 * Reads 4 bytes from array starting at offset as big-endian
 * signed 32-bit integer and returns it.
 */
export declare function readInt32BE(array: Uint8Array, offset?: number): number;
/**
 * Reads 4 bytes from array starting at offset as big-endian
 * unsigned 32-bit integer and returns it.
 */
export declare function readUint32BE(array: Uint8Array, offset?: number): number;
/**
 * Reads 4 bytes from array starting at offset as little-endian
 * signed 32-bit integer and returns it.
 */
export declare function readInt32LE(array: Uint8Array, offset?: number): number;
/**
 * Reads 4 bytes from array starting at offset as little-endian
 * unsigned 32-bit integer and returns it.
 */
export declare function readUint32LE(array: Uint8Array, offset?: number): number;
/**
 * Writes 4-byte big-endian representation of 32-bit unsigned
 * value to byte array starting at offset.
 *
 * If byte array is not given, creates a new 4-byte one.
 *
 * Returns the output byte array.
 */
export declare function writeUint32BE(value: number, out?: Uint8Array, offset?: number): Uint8Array;
export declare const writeInt32BE: typeof writeUint32BE;
/**
 * Writes 4-byte little-endian representation of 32-bit unsigned
 * value to array starting at offset.
 *
 * If byte array is not given, creates a new 4-byte one.
 *
 * Returns the output byte array.
 */
export declare function writeUint32LE(value: number, out?: Uint8Array, offset?: number): Uint8Array;
export declare const writeInt32LE: typeof writeUint32LE;
/**
 * Reads 8 bytes from array starting at offset as big-endian
 * signed 64-bit integer and returns it.
 *
 * IMPORTANT: due to JavaScript limitation, supports exact
 * numbers in range -9007199254740991 to 9007199254740991.
 * If the number stored in the byte array is outside this range,
 * the result is not exact.
 */
export declare function readInt64BE(array: Uint8Array, offset?: number): number;
/**
 * Reads 8 bytes from array starting at offset as big-endian
 * unsigned 64-bit integer and returns it.
 *
 * IMPORTANT: due to JavaScript limitation, supports values up to 2^53-1.
 */
export declare function readUint64BE(array: Uint8Array, offset?: number): number;
/**
 * Reads 8 bytes from array starting at offset as little-endian
 * signed 64-bit integer and returns it.
 *
 * IMPORTANT: due to JavaScript limitation, supports exact
 * numbers in range -9007199254740991 to 9007199254740991.
 * If the number stored in the byte array is outside this range,
 * the result is not exact.
 */
export declare function readInt64LE(array: Uint8Array, offset?: number): number;
/**
 * Reads 8 bytes from array starting at offset as little-endian
 * unsigned 64-bit integer and returns it.
 *
 * IMPORTANT: due to JavaScript limitation, supports values up to 2^53-1.
 */
export declare function readUint64LE(array: Uint8Array, offset?: number): number;
/**
 * Writes 8-byte big-endian representation of 64-bit unsigned
 * value to byte array starting at offset.
 *
 * Due to JavaScript limitation, supports values up to 2^53-1.
 *
 * If byte array is not given, creates a new 8-byte one.
 *
 * Returns the output byte array.
 */
export declare function writeUint64BE(value: number, out?: Uint8Array, offset?: number): Uint8Array;
export declare const writeInt64BE: typeof writeUint64BE;
/**
 * Writes 8-byte little-endian representation of 64-bit unsigned
 * value to byte array starting at offset.
 *
 * Due to JavaScript limitation, supports values up to 2^53-1.
 *
 * If byte array is not given, creates a new 8-byte one.
 *
 * Returns the output byte array.
 */
export declare function writeUint64LE(value: number, out?: Uint8Array, offset?: number): Uint8Array;
export declare const writeInt64LE: typeof writeUint64LE;
/**
 * Reads bytes from array starting at offset as big-endian
 * unsigned bitLen-bit integer and returns it.
 *
 * Supports bit lengths divisible by 8, up to 48.
 */
export declare function readUintBE(bitLength: number, array: Uint8Array, offset?: number): number;
/**
 * Reads bytes from array starting at offset as little-endian
 * unsigned bitLen-bit integer and returns it.
 *
 * Supports bit lengths divisible by 8, up to 48.
 */
export declare function readUintLE(bitLength: number, array: Uint8Array, offset?: number): number;
/**
 * Writes a big-endian representation of bitLen-bit unsigned
 * value to array starting at offset.
 *
 * Supports bit lengths divisible by 8, up to 48.
 *
 * If byte array is not given, creates a new one.
 *
 * Returns the output byte array.
 */
export declare function writeUintBE(bitLength: number, value: number, out?: Uint8Array, offset?: number): Uint8Array;
/**
 * Writes a little-endian representation of bitLen-bit unsigned
 * value to array starting at offset.
 *
 * Supports bit lengths divisible by 8, up to 48.
 *
 * If byte array is not given, creates a new one.
 *
 * Returns the output byte array.
 */
export declare function writeUintLE(bitLength: number, value: number, out?: Uint8Array, offset?: number): Uint8Array;
/**
 * Reads 4 bytes from array starting at offset as big-endian
 * 32-bit floating-point number and returns it.
 */
export declare function readFloat32BE(array: Uint8Array, offset?: number): number;
/**
 * Reads 4 bytes from array starting at offset as little-endian
 * 32-bit floating-point number and returns it.
 */
export declare function readFloat32LE(array: Uint8Array, offset?: number): number;
/**
 * Reads 8 bytes from array starting at offset as big-endian
 * 64-bit floating-point number ("double") and returns it.
 */
export declare function readFloat64BE(array: Uint8Array, offset?: number): number;
/**
 * Reads 8 bytes from array starting at offset as little-endian
 * 64-bit floating-point number ("double") and returns it.
 */
export declare function readFloat64LE(array: Uint8Array, offset?: number): number;
/**
 * Writes 4-byte big-endian floating-point representation of value
 * to byte array starting at offset.
 *
 * If byte array is not given, creates a new 4-byte one.
 *
 * Returns the output byte array.
 */
export declare function writeFloat32BE(value: number, out?: Uint8Array, offset?: number): Uint8Array;
/**
 * Writes 4-byte little-endian floating-point representation of value
 * to byte array starting at offset.
 *
 * If byte array is not given, creates a new 4-byte one.
 *
 * Returns the output byte array.
 */
export declare function writeFloat32LE(value: number, out?: Uint8Array, offset?: number): Uint8Array;
/**
 * Writes 8-byte big-endian floating-point representation of value
 * to byte array starting at offset.
 *
 * If byte array is not given, creates a new 8-byte one.
 *
 * Returns the output byte array.
 */
export declare function writeFloat64BE(value: number, out?: Uint8Array, offset?: number): Uint8Array;
/**
 * Writes 8-byte little-endian floating-point representation of value
 * to byte array starting at offset.
 *
 * If byte array is not given, creates a new 8-byte one.
 *
 * Returns the output byte array.
 */
export declare function writeFloat64LE(value: number, out?: Uint8Array, offset?: number): Uint8Array;
