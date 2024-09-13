type GenericEmitterType = {
    [key: string]: unknown;
};
export type Emitter<T extends GenericEmitterType> = {
    subscribe<K extends keyof T>(event: K, cb: (data: T[K]) => void): () => void;
    emit<K extends keyof T>(event: K, data: T[K]): void;
};
/**
 * Creates an emitter object that allows subscribing to events and emitting events.
 * @returns An emitter object with `subscribe` and `emit` methods.
 * @template TEmitter - The type of the emitter.
 * @example
 * ```ts
 * const emitter = createEmitter<{
 *  event1: string;
 * event2: number;
 * }>();
 *
 * emitter.subscribe("event1", (data) => {
 * console.log(data); // "hello"
 * });
 *
 * emitter.emit("event1", "hello");
 * ```
 */
export declare function createEmitter<const TEmitter extends GenericEmitterType>(): Emitter<TEmitter>;
export {};
//# sourceMappingURL=tiny-emitter.d.ts.map