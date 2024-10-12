import * as React from 'react';
import { Primitive } from '@radix-ui/react-primitive';

type PrimitiveDivProps = React.ComponentPropsWithoutRef<typeof Primitive.div>;
interface PortalProps extends PrimitiveDivProps {
    /**
     * An optional container where the portaled content should be appended.
     */
    container?: Element | null;
}
declare const Portal: React.ForwardRefExoticComponent<PortalProps & React.RefAttributes<HTMLDivElement>>;
declare const Root: React.ForwardRefExoticComponent<PortalProps & React.RefAttributes<HTMLDivElement>>;

export { Portal, type PortalProps, Root };
