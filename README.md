

## ⚔️ Kill The Wallet

💫💫💫 **we would be very grateful if you could take 5 seconds to star our repository on Github. It helps get the word out to more crypto, web3 and Flutter developers & open source committers about WootzApp** 💫💫💫

- 👐 **Open Source**. We have the exact same license as Metamask
- 🎁 **No Extensions**. Just use the damn browser!
- 🙈 **Privacy First**. Not even ads.
- 🔐 **Cryptographic storage**. We use [Flutter Secure Store](https://pub.dev/packages/flutter_secure_storage) for secure storage on mobile. On Android and ios, it uses the native encrypted storage (like secure enclave)

## 🧬What is the meaning of "Wootz"


[Wootz steel](https://en.wikipedia.org/wiki/Wootz_steel) originated in South India - in present day Kodumanal, Tamil Nadu. The word “*wootz*” appears to have originated as a mistranscription of *wook* - from the Tamil language root word *urukku*.  
Trade between South India and Sri Lanka through the Arabian Sea introduced wootz steel to Arabia. It spread the technology to the city of Damascus, where an industry developed for making weapons of this steel..and began to be known as Damascus steel (much like mathematics!)

After [India's First War of Independence of 1857](https://en.wikipedia.org/wiki/Indian_Rebellion_of_1857), the colonial British government banned weapon-making by Indians. This led to the stoppage of the production of Wootz. And the secret was lost forever.

**Wootz is the strongest, sharpest and best steel in the world. Definitely better than [Chrome](https://en.wikipedia.org/wiki/Chrome_steel) ;)**

## 🎊 Community

- Install our [Android app](https://play.google.com/store/apps/details?id=com.wootzapp.browser)
- Join our [Discord](https://discord.gg/GjUfSqzmzx). Developers and non-developers both are welcome. Designers are VERY welcome.
- Follow our [Twitter](https://twitter.com/wootzapp)
- Join  [support Telegram](https://t.me/wootzapp) to get help, or to chat with us!!!
- Open a [discussion](https://github.com/wootzapp/wootz-browser/discussions/new) with your question, or
- Open [a bug](https://github.com/wootzapp/wootz-browser/issues/new)
- Our [Figma](https://www.figma.com/file/ZiV1e5dTxNU20so3foISK7/wootzapp)
- Check open [Github Issues](https://github.com/wootzapp/wootz-browser/issues)
- Make sure to read our [contributing CLA](https://github.com/wootzapp/.github/blob/main/CLA.md).

## 🪅 Getting Started

1. It's Flutter!

## 🧐 Contributing Guidelines (There is only one)

This project hopes and requests for clean pull request merges. the way we merge is squash and merge. This fundamentally can only work if you **NEVER ISSUE A PULL REQUEST TWICE FROM THE SAME LOCAL BRANCH**. If you create another pull request from same local branch, then the merge will always fail.

solution is simple - **ONE BRANCH PER PULL REQUEST**. We Follow this strictly. if you have created this pull request using your master/main branch, then follow these steps to fix it:
```
# Note: Any changes not committed will be lost.
git branch newbranch      # Create a new branch, saving the desired commits
git checkout master       # checkout master, this is the place you want to go back (which will be same as the last commit in WootzApp main repo)
git reset --hard HEAD~3   # Move master back by 3 commits (Make sure you know how many commits you need to go back)
git checkout newbranch    # Go to the new branch that still has the desired commits. NOW CREATE A PULL REQUEST
```

## 💌 Acknowledgements

- First Hat tip to  [Metamask](https://github.com/MetaMask).
- We continue the spirit of [Netscape](https://www.cnet.com/tech/tech-industry/netscape-sets-source-code-free/).
- The Extension Ethereum Provider is a fork of [Alphawallet Provider](https://github.com/AlphaWallet/AlphaWallet-web3-provider).
- Original version was inspired by [flutter_inappwebview browser](https://github.com/pichillilorenzo/flutter_browser_app)
- All the other [contributors](https://github.com/wootzapp/wootz-browser/graphs/contributors).

## ✍️ Authors and Contributors

- Sandeep Srinivasa ([@sandys](https://twitter.com/sandeepssrin))
- Davy King ([@Imdavyking](https://github.com/Imdavyking))

We love contributors! Feel free to contribute to this project but please read the [CLA](https://github.com/wootzapp/.github/blob/main/CLA.md) first!

<a href="https://github.com/wootzapp/wootz-browser/graphs/contributors">
  <img src="https://contrib.rocks/image?repo=wootzapp/wootz-browser&max=300&columns=12&anon=0" />
</a>

## 📜 License

WootzApp is open-source OSS software. We are not lawyers, hence we have adopted the exact same license that Metamask does. Their reasoning is mentioned here - https://medium.com/metamask/evolving-our-license-for-the-next-wave-of-metamask-users-81b0b494c09a



## ❤️ Old Changelog



This is a Mobile crypto wallet, developed with flutter for IOS and Android.

add solana to total balance ✅  
add solana transactions ✅  
make qr code also use gallery and torch light ✅  
fix transaction fee on sign transaction modal ✅  
pause camera when app in background ✅  
added filecoin ✅  

autheniticate on pause. ✅  
localization, add more languages. ✅  
ens support ✅  
block explorer closure can not be json encoded fixed ✅  
add stellar blockchain ✅  
add support for wallet connect and ethereum links externally. ✅  
decode known abis.  

check intent url link opening wc: wallet connect url on ios.  

fix cardano ✅  
set changeaddress = firstunused ✅  
so the unspent will be transferred to user wallet. ✅  

Class '\_InternalLinkedHashMap<String, Object>' ---> find more about this. ✅  

change minimum transaction to save automatically in transfer token.  

fix swap page. ✅  

0x095ea7b300000000000000000000000015eb350807cd1173fb88dc536c951a743811636000000000000000000000000000000000000000000000000029a2241af62c0000  

to -> "0x15EB350807cd1173fb88DC536c951a7438116360", "3"  

decode this.  

fix cosmos sending...  
change lcdurl for cosmos to sdk 0.37.9 / cosmoshub-3  
find a lcd with this version.  

update xcode to version 14 for run app.  
fix userscripts not working on new tab  
