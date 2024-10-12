import './App.css';

import { chain } from '@/shared/chain'
import { client } from '@/shared/client'
import { inAppWallet } from 'thirdweb/wallets'

function App() {

 async function loadWallet(token) {
  const wallet = inAppWallet()

  return (
    wallet
      //@ts-expect-error temporary
      .connect({
        client,
        chain,
        strategy: 'jwt',
        jwt: token,
      })
  )
}
  return (
    <div className="App">
      <header className="App-header">
        <p>
          Edit <code>src/App.js</code> and save to reload.
        </p>
        <a
          className="App-link"
          href="https://reactjs.org"
          target="_blank"
          rel="noopener noreferrer"
        >
          Learn React
        </a>
        {loadWallet("eyJhbGciOiJSUzI1NiIsImtpZCI6ImMxNTQwYWM3MWJiOTJhYTA2OTNjODI3MTkwYWNhYmU1YjA1NWNiZWMiLCJ0eXAiOiJKV1QifQ.eyJpc3MiOiJodHRwczovL3NlY3VyZXRva2VuLmdvb2dsZS5jb20vYWR0ZXN0LTk2YWJlIiwiYXVkIjoiYWR0ZXN0LTk2YWJlIiwiYXV0aF90aW1lIjoxNzIxMzk2Mjc1LCJ1c2VyX2lkIjoiR2VwRm1wVE5qQ1hIcVNzeXJJcW5hVmowUUU2MiIsInN1YiI6IkdlcEZtcFROakNYSHFTc3lySXFuYVZqMFFFNjIiLCJpYXQiOjE3MjEzOTYyNzUsImV4cCI6MTcyMTM5OTg3NSwiZW1haWwiOiJqYXlhZG1pbkBnbWFpbC5jb20iLCJlbWFpbF92ZXJpZmllZCI6ZmFsc2UsImZpcmViYXNlIjp7ImlkZW50aXRpZXMiOnsiZW1haWwiOlsiamF5YWRtaW5AZ21haWwuY29tIl19LCJzaWduX2luX3Byb3ZpZGVyIjoicGFzc3dvcmQifX0.AU37zlRDpTaUVrPN8VLAXjEb_HFRywHXgGfla1B2Tduo0q3BNL_orDB8sKL9eqsUa6fWxBsFS1VZx0pEjV_ExxHr3PtQ7A5zEZwu_TgqGJ-EwsdM7hoZ0JBLQz7bLLh0fMkZMdEmKahKF8wPOtu11vpoMB2cwp750eMdVPkRQKG_ppbqVcIIkmrBiFpxIVnH-s7CJzqgOsM_GPj5jyuIv7oCCUFNoR7sV2Brd1PilDdM0ccrnwT3-T3awe2riJTIkP1-g2PkP878v3Yee20y-kqECViD4Dk-UKtOsmeMFgPQCP2CtgxezNZB16WRO5-zaArhRyxGAYk122RN4frHyA")}
      </header>
    </div>
  );
}

export default App;
