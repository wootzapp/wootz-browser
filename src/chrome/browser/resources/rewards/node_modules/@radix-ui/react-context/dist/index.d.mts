import * as react_jsx_runtime from 'react/jsx-runtime';
import * as React from 'react';

declare function createContext<ContextValueType extends object | null>(rootComponentName: string, defaultContext?: ContextValueType): readonly [{
    (props: ContextValueType & {
        children: React.ReactNode;
    }): react_jsx_runtime.JSX.Element;
    displayName: string;
}, (consumerName: string) => ContextValueType];
type Scope<C = any> = {
    [scopeName: string]: React.Context<C>[];
} | undefined;
type ScopeHook = (scope: Scope) => {
    [__scopeProp: string]: Scope;
};
interface CreateScope {
    scopeName: string;
    (): ScopeHook;
}
declare function createContextScope(scopeName: string, createContextScopeDeps?: CreateScope[]): readonly [<ContextValueType extends object | null>(rootComponentName: string, defaultContext?: ContextValueType | undefined) => readonly [{
    (props: ContextValueType & {
        scope: Scope<ContextValueType>;
        children: React.ReactNode;
    }): react_jsx_runtime.JSX.Element;
    displayName: string;
}, (consumerName: string, scope: Scope<ContextValueType | undefined>) => ContextValueType], CreateScope];

export { type CreateScope, type Scope, createContext, createContextScope };
