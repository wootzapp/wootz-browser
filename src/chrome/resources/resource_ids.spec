# Since recent changes upstream, Chromium is now using IDs up to ~46310
# (see end of //out/Component/gen/tools/gritsettings/default_resource_ids),
# so let's leave some padding after that and start assigning them on 47500.
{
  "SRCDIR": "../..",
  "chrome/common/extensions/api/wootz_api_resources.grd": {
    "includes": [58700],
  },
  "components/resources/wootz_components_resources.grd": {
    "includes": [58710],
  },
 
  # This file is generated during the build.
  "<(SHARED_INTERMEDIATE_DIR)/chrome/web-ui-wootz_new_tab/wootz_new_tab.grd": {
    "META": {"sizes": {"includes": [50]}},
    "includes": [58910],
  },
  # This file is generated during the build.
  "<(SHARED_INTERMEDIATE_DIR)/chrome/web-ui-wootz_welcome/wootz_welcome.grd": {
    "META": {"sizes": {"includes": [20]}},
    "includes": [58960],
  },
  # This file is generated during the build.
  "<(SHARED_INTERMEDIATE_DIR)/chrome/browser/resources/settings/wootz_settings_resources.grd": {
    "META": {"sizes": {"includes": [20]}},
    "includes": [58980],
  },
  "chrome/app/wootz_generated_resources.grd": {
    "includes": [59000],
    "messages": [59050],
  },
  "components/resources/wootz_components_strings.grd": {
    "messages": [60210],
  },

  # This file is generated during the build.
  "<(SHARED_INTERMEDIATE_DIR)/chrome/web-ui-wootz_wallet_page/wootz_wallet_page.grd": {
    "META": {"sizes": {"includes": [200]}},
    "includes": [62760],
  },
  # This file is generated during the build.
  "<(SHARED_INTERMEDIATE_DIR)/chrome/web-ui-wootz_wallet_panel/wootz_wallet_panel.grd": {
    "META": {"sizes": {"includes": [200]}},
    "includes": [62970],
  },

  # This file is generated during the build.
  "<(SHARED_INTERMEDIATE_DIR)/chrome/web-ui-wootz_wallet_script/wootz_wallet_script.grd": {
    "META": {"sizes": {"includes": [10]}},
    "includes": [63320],
  },

  # This file is generated during the build.
  "<(SHARED_INTERMEDIATE_DIR)/chrome/web-ui-trezor_bridge/trezor_bridge.grd": {
    "META": {"sizes": {"includes": [10]}},
    "includes": [63350],
  },
  # This file is generated during the build.
  "<(SHARED_INTERMEDIATE_DIR)/chrome/web-ui-market_display/market_display.grd": {
    "META": {"sizes": {"includes": [100]}},
    "includes": [63360],
  },

  "<(SHARED_INTERMEDIATE_DIR)/chrome/web-ui-ledger_bridge/ledger_bridge.grd": {
    "META": {"sizes": {"includes": [10]}},
    "includes": [63480],
  },
  # This file is generated during the build.
  "<(SHARED_INTERMEDIATE_DIR)/chrome/web-ui-nft_display/nft_display.grd": {
    "META": {"sizes": {"includes": [90]}},
    "includes": [63490],
  },
  "<(SHARED_INTERMEDIATE_DIR)/chrome/web-ui-cookie_list_opt_in/cookie_list_opt_in.grd": {
    "META": {"sizes": {"includes": [10]}},
    "includes": [63580],
  },

  "<(SHARED_INTERMEDIATE_DIR)/chrome/web-ui-commands/commands.grd": {
    "META": {"sizes": {"includes": [10]}},
    "includes": [63610],
  },
  "<(SHARED_INTERMEDIATE_DIR)/chrome/web-ui-wootz_wallet_swap_page/wootz_wallet_swap_page.grd": {
    "META": {"sizes": {"includes": [150]}},
    "includes": [63620],
  },
  "<(SHARED_INTERMEDIATE_DIR)/chrome/web-ui-wootz_wallet_send_page/wootz_wallet_send_page.grd": {
    "META": {"sizes": {"includes": [100]}},
    "includes": [63770],
  },
  "<(SHARED_INTERMEDIATE_DIR)/chrome/web-ui-wootz_wallet_deposit_page/wootz_wallet_deposit_page.grd": {
    "META": {"sizes": {"includes": [100]}},
    "includes": [63870],
  },
  "<(SHARED_INTERMEDIATE_DIR)/chrome/web-ui-wootz_wallet_fund_wallet_page/wootz_wallet_fund_wallet_page.grd": {
    "META": {"sizes": {"includes": [100]}},
    "includes": [63970],
  },

}
