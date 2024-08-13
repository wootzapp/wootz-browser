# Since recent changes upstream, Chromium is now using IDs up to ~46310
# (see end of //out/Component/gen/tools/gritsettings/default_resource_ids),
# so let's leave some padding after that and start assigning them on 47500.
{
  "SRCDIR": "../..",
  "chrome/common/extensions/api/wootz_api_resources.grd": {
    "includes": [57400],
  },
  "chrome/components/resources/wootz_components_resources.grd": {
    "includes": [57410],
  },
 
  # This file is generated during the build.
  "<(SHARED_INTERMEDIATE_DIR)/wootz/web-ui-wootz_new_tab/wootz_new_tab.grd": {
    "META": {"sizes": {"includes": [50]}},
    "includes": [57620],
  },
  # This file is generated during the build.
  "<(SHARED_INTERMEDIATE_DIR)/wootz/web-ui-wootz_welcome/wootz_welcome.grd": {
    "META": {"sizes": {"includes": [20]}},
    "includes": [57670],
  },
  # This file is generated during the build.
  "<(SHARED_INTERMEDIATE_DIR)/wootz/browser/resources/settings/wootz_settings_resources.grd": {
    "META": {"sizes": {"includes": [20]}},
    "includes": [57690],
  },
  "chrome/app/wootz_generated_resources.grd": {
    "includes": [57700],
    "messages": [57750],
  },
  "chrome/components/resources/wootz_components_strings.grd": {
    "messages": [60210],
  },

  # This file is generated during the build.
  "<(SHARED_INTERMEDIATE_DIR)/wootz/web-ui-wootz_wallet_page/wootz_wallet_page.grd": {
    "META": {"sizes": {"includes": [200]}},
    "includes": [61460],
  },
  # This file is generated during the build.
  "<(SHARED_INTERMEDIATE_DIR)/wootz/web-ui-ethereum_remote_client_page/ethereum_remote_client_page.grd": {
    "META": {"sizes": {"includes": [10]}},
    "includes": [61660],
  },
  # This file is generated during the build.
  "<(SHARED_INTERMEDIATE_DIR)/wootz/web-ui-wootz_wallet_panel/wootz_wallet_panel.grd": {
    "META": {"sizes": {"includes": [200]}},
    "includes": [61670],
  },

  # This file is generated during the build.
  "<(SHARED_INTERMEDIATE_DIR)/wootz/web-ui-wootz_wallet_script/wootz_wallet_script.grd": {
    "META": {"sizes": {"includes": [10]}},
    "includes": [62030],
  },

  # This file is generated during the build.
  "<(SHARED_INTERMEDIATE_DIR)/wootz/web-ui-trezor_bridge/trezor_bridge.grd": {
    "META": {"sizes": {"includes": [10]}},
    "includes": [62060],
  },
  # This file is generated during the build.
  "<(SHARED_INTERMEDIATE_DIR)/wootz/web-ui-market_display/market_display.grd": {
    "META": {"sizes": {"includes": [100]}},
    "includes": [62070],
  },

  "<(SHARED_INTERMEDIATE_DIR)/wootz/web-ui-ledger_bridge/ledger_bridge.grd": {
    "META": {"sizes": {"includes": [10]}},
    "includes": [62190],
  },
  # This file is generated during the build.
  "<(SHARED_INTERMEDIATE_DIR)/wootz/web-ui-nft_display/nft_display.grd": {
    "META": {"sizes": {"includes": [90]}},
    "includes": [62200],
  },
  "<(SHARED_INTERMEDIATE_DIR)/wootz/web-ui-cookie_list_opt_in/cookie_list_opt_in.grd": {
    "META": {"sizes": {"includes": [10]}},
    "includes": [62290],
  },

  "<(SHARED_INTERMEDIATE_DIR)/wootz/web-ui-commands/commands.grd": {
    "META": {"sizes": {"includes": [10]}},
    "includes": [62320],
  },
  "<(SHARED_INTERMEDIATE_DIR)/wootz/web-ui-wootz_wallet_swap_page/wootz_wallet_swap_page.grd": {
    "META": {"sizes": {"includes": [150]}},
    "includes": [62330],
  },
  "<(SHARED_INTERMEDIATE_DIR)/wootz/web-ui-wootz_wallet_send_page/wootz_wallet_send_page.grd": {
    "META": {"sizes": {"includes": [100]}},
    "includes": [62470],
  },
  "<(SHARED_INTERMEDIATE_DIR)/wootz/web-ui-wootz_wallet_deposit_page/wootz_wallet_deposit_page.grd": {
    "META": {"sizes": {"includes": [100]}},
    "includes": [62570],
  },
  "<(SHARED_INTERMEDIATE_DIR)/wootz/web-ui-wootz_wallet_fund_wallet_page/wootz_wallet_fund_wallet_page.grd": {
    "META": {"sizes": {"includes": [100]}},
    "includes": [62670],
  },
  
}
