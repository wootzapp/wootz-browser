export default class LogLinkedList {
    private lengthInNodes;
    private sizeInBytes;
    private head;
    private tail;
    private maxSizeInBytes;
    constructor(maxSizeInBytes: number);
    append(value: string): void;
    shift(): void;
    toArray(): string[];
    get length(): number;
    get size(): number;
    toOrderedArray(): string[];
    [Symbol.iterator](): Iterator<string>;
}
//# sourceMappingURL=linkedList.d.ts.map