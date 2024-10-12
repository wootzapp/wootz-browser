import { ReactiveElement } from 'lit';
import { Intrinsics } from './styles/evaluators';
import { Unit } from './styles/parsers';
export declare type IntrinsicsFactory<T extends Intrinsics<Array<Unit>>, U extends ReactiveElement> = (element: U) => T;
export interface StyleDecoratorConfig<T extends Intrinsics<Array<Unit>>, U extends ReactiveElement> {
    intrinsics: T | IntrinsicsFactory<T, U>;
    updateHandler: symbol;
    observeEffects?: boolean;
}
/**
 * The @style decorator is responsible for coordinating the conversion of a
 * CSS-like string property value into numbers that can be applied to
 * lower-level constructs. It also can optionally manage the lifecycle of a
 * StyleEffector which allows automatic updates for styles that use env() or
 * var() functions.
 *
 * The decorator is configured with Intrinsics and the property key for a
 * method that handles updates. The named update handler is invoked with the
 * result of parsing and evaluating the raw property string value. The format of
 * the evaluated result is derived from the basis of the configured Intrinsics,
 * and is always an array of numbers of fixed length.
 *
 * NOTE: This decorator depends on the property updating mechanism defined by
 * UpdatingElement as exported by the lit-element module. That means it *must*
 * be used in conjunction with the @property decorator, or equivalent
 * JavaScript.
 *
 * Supported configurations are:
 *
 *  - `intrinsics`: An Intrinsics struct that describes how to interpret a
 * serialized style attribute. For more detail on intrinsics see
 * ./styles/evaluators.ts
 *  - `updateHandler`: A string or Symbol that is the key of a method to be
 * invoked with the result of parsing and evaluating a serialized style string.
 *  - `observeEffects`: Optional, if set to true then styles that use env() will
 * cause their update handlers to be invoked every time the corresponding
 * environment variable changes (even if the style attribute itself remains
 * static).
 */
export declare const style: <T extends Intrinsics<Unit[]>, U extends ReactiveElement>(config: StyleDecoratorConfig<T, U>) => <U_1 extends ReactiveElement>(proto: U_1, propertyName: string) => void;
