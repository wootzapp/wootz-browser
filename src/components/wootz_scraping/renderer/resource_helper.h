/* Copyright (c) 2024 All rights reserved. */

#ifndef COMPONENTS_wootz_scraping_RENDERER_RESOURCE_HELPER_H_
#define COMPONENTS_wootz_scraping_RENDERER_RESOURCE_HELPER_H_

#include <string>

namespace blink {
struct WebScriptSource;
}

namespace wootz_scraping {

// Get the Twitter interceptor script as a string
const char* GetTwitterInterceptorScript();

// Get a WebScriptSource for the Twitter interceptor script
blink::WebScriptSource GetTwitterInterceptorScriptSource();

}  // namespace wootz_scraping

#endif  // COMPONENTS_wootz_scraping_RENDERER_RESOURCE_HELPER_H_