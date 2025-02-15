import React from 'react';
import ExtensionCard from './ExtensionCard';

function ExtensionList({ extensions }) {
  return (
    <div className="flex flex-col gap-4 w-full">
      {extensions.map((extension) => (
        <ExtensionCard key={extension.id} extension={extension} />
      ))}
    </div>
  );
}

export default ExtensionList; 