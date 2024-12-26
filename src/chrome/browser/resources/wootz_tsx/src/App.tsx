import { ConnectButton } from "thirdweb/react";
import { client } from "./client";

export default function App() {
	return (
		<main className="p-4 pb-10 min-h-[100vh] flex items-center justify-center container max-w-screen-lg mx-auto">
			<div className="py-20">
				<Header />

				<div className="flex justify-center mb-20">
					<ConnectButton
						client={client}
						appMetadata={{
							name: "Example app",
							url: "https://example.com",
						}}
					/>
				</div>

				<ThirdwebResources />
			</div>
		</main>
	);
}

function Header() {
	return (
		<header className="flex flex-col items-center mb-20 md:mb-20">
			

			<h1 className="text-2xl md:text-6xl font-bold tracking-tighter mb-6 text-zinc-100">
				thirdweb SDK
				<span className="text-zinc-300 inline-block mx-1"> + </span>
				<span className="inline-block -skew-x-6 text-violet-500"> vite </span>
			</h1>

			<p className="text-zinc-300 text-base">
				Read the{" "}
				<code className="bg-zinc-800 text-zinc-300 px-2 rounded py-1 text-sm mx-1">
					README.md
				</code>{" "}
				file to get started.
			</p>
		</header>
	);
}

function ThirdwebResources() {
	return (
		<div className="grid gap-4 lg:grid-cols-3 justify-center">
			<ArticleCard
				title="thirdweb SDK Docs"
				href="https://portal.thirdweb.com/typescript/v5"
				description="thirdweb TypeScript SDK documentation"
			/>

			<ArticleCard
				title="Components and Hooks"
				href="https://portal.thirdweb.com/typescript/v5/react"
				description="Learn about the thirdweb React components and hooks in thirdweb SDK"
			/>

			<ArticleCard
				title="thirdweb Dashboard"
				href="https://thirdweb.com/dashboard"
				description="Deploy, configure, and manage your smart contracts from the dashboard."
			/>
		</div>
	);
}

function ArticleCard(props: {
	title: string;
	href: string;
	description: string;
}) {
	return (
		<a
			href={`${props.href}?utm_source=vite-template`}
			target="_blank"
			className="flex flex-col border border-zinc-800 p-4 rounded-lg hover:bg-zinc-900 transition-colors hover:border-zinc-700"
			rel="noreferrer"
		>
			<article>
				<h2 className="text-lg font-semibold mb-2">{props.title}</h2>
				<p className="text-sm text-zinc-400">{props.description}</p>
			</article>
		</a>
	);
}