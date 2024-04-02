// Copyright 2016 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chromeos/printing/printer_translator.h"

#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

#include "base/logging.h"
#include "base/strings/string_util.h"
#include "base/strings/stringprintf.h"
#include "base/values.h"
#include "chromeos/printing/cups_printer_status.h"
#include "chromeos/printing/printer_configuration.h"
#include "chromeos/printing/uri.h"
#include "url/gurl.h"
#include "url/url_constants.h"

namespace chromeos {

namespace {

// For historical reasons, the effective_make_and_model field is just
// effective_model for policy printers.
const char kEffectiveModel[] = "effective_model";

// printer fields
const char kDisplayName[] = "display_name";
const char kDescription[] = "description";
const char kManufacturer[] = "manufacturer";
const char kModel[] = "model";
const char kUri[] = "uri";
const char kUUID[] = "uuid";
const char kPpdResource[] = "ppd_resource";
const char kAutoconf[] = "autoconf";
const char kGuid[] = "guid";
const char kUserSuppliedPpdUri[] = "user_supplied_ppd_uri";

// Populates the |printer| object with corresponding fields from |value|.
// Returns false if |value| is missing a required field.
bool DictionaryToPrinter(const base::Value::Dict& value, Printer* printer) {
  // Mandatory fields
  const std::string* display_name = value.FindString(kDisplayName);
  if (!display_name) {
    LOG(WARNING) << "Display name required";
    return false;
  }
  printer->set_display_name(*display_name);

  const std::string* uri = value.FindString(kUri);
  if (!uri) {
    LOG(WARNING) << "Uri required";
    return false;
  }

  std::string message;
  if (!printer->SetUri(*uri, &message)) {
    LOG(WARNING) << message;
    return false;
  }

  // Optional fields
  const std::string* description = value.FindString(kDescription);
  if (description)
    printer->set_description(*description);

  const std::string* manufacturer = value.FindString(kManufacturer);
  const std::string* model = value.FindString(kModel);

  std::string make_and_model = manufacturer ? *manufacturer : std::string();
  if (!make_and_model.empty() && model && !model->empty())
    make_and_model.append(" ");
  if (model)
    make_and_model.append(*model);
  printer->set_make_and_model(make_and_model);

  const std::string* uuid = value.FindString(kUUID);
  if (uuid)
    printer->set_uuid(*uuid);

  return true;
}

// Create an empty CupsPrinterInfo dictionary value. It should be consistent
// with the fields in js side. See cups_printers_browser_proxy.js for the
// definition of CupsPrintersInfo.
base::Value::Dict CreateEmptyPrinterInfo() {
  base::Value::Dict printer_info;
  printer_info.Set("isManaged", false);
  printer_info.Set("ppdManufacturer", "");
  printer_info.Set("ppdModel", "");
  printer_info.Set("printerAddress", "");
  printer_info.SetByDottedPath("printerPpdReference.autoconf", false);
  printer_info.Set("printerDescription", "");
  printer_info.Set("printerId", "");
  printer_info.Set("printerMakeAndModel", "");
  printer_info.Set("printerName", "");
  printer_info.Set("printerPPDPath", "");
  printer_info.Set("printerProtocol", "ipp");
  printer_info.Set("printerQueue", "");
  return printer_info;
}

// Formats a host and port string. The |port| portion is omitted if it is
// unspecified or invalid.
std::string PrinterAddress(const Uri& uri) {
  const int port = uri.GetPort();
  if (port > -1) {
    return base::StringPrintf("%s:%d", uri.GetHostEncoded().c_str(), port);
  }
  return uri.GetHostEncoded();
}

bool ValidateAndSetPpdReference(const base::Value::Dict& ppd_resource,
                                Printer& printer) {
  std::optional<bool> autoconf = ppd_resource.FindBool(kAutoconf);
  const std::string* effective_model = ppd_resource.FindString(kEffectiveModel);
  const std::string* user_supplied_ppd_uri =
      ppd_resource.FindString(kUserSuppliedPpdUri);

  const bool is_autoconf = autoconf.value_or(false);
  const bool has_effective_model = effective_model && !effective_model->empty();
  const bool has_user_supplied_ppd_uri =
      user_supplied_ppd_uri && !user_supplied_ppd_uri->empty();

  const bool has_exactly_one_ppd_resource =
      is_autoconf + has_effective_model + has_user_supplied_ppd_uri == 1;
  if (!has_exactly_one_ppd_resource) {
    LOG(WARNING) << base::StringPrintf(
        "Managed printer '%s' must have exactly one %s value: is_autoconf: %d, "
        "has_effective_model: %d, has_user_supplied_ppd_uri: %d",
        printer.display_name().c_str(), kPpdResource, is_autoconf,
        has_effective_model, has_user_supplied_ppd_uri);
    return false;
  }

  if (is_autoconf) {
    printer.mutable_ppd_reference()->autoconf = true;
  }
  if (has_effective_model) {
    printer.mutable_ppd_reference()->effective_make_and_model =
        *effective_model;
  }
  if (has_user_supplied_ppd_uri) {
    GURL url(*user_supplied_ppd_uri);
    if (!url.is_valid() || !url.SchemeIsHTTPOrHTTPS()) {
      LOG(WARNING) << base::StringPrintf(
          "Managed printer '%s' has invalid %s.%s: '%s'",
          printer.display_name().c_str(), kPpdResource, kUserSuppliedPpdUri,
          user_supplied_ppd_uri->c_str());
      return false;
    }
    printer.mutable_ppd_reference()->user_supplied_ppd_url =
        *user_supplied_ppd_uri;
  }

  return true;
}

}  // namespace

const char kPrinterId[] = "id";

std::unique_ptr<Printer> RecommendedPrinterToPrinter(
    const base::Value::Dict& pref) {
  std::string id;
  // Printer id comes from the id or guid field depending on the source.
  const std::string* printer_id = pref.FindString(kPrinterId);
  const std::string* printer_guid = pref.FindString(kGuid);
  if (printer_id) {
    id = *printer_id;
  } else if (printer_guid) {
    id = *printer_guid;
  } else {
    LOG(WARNING) << "Record id required";
    return nullptr;
  }

  auto printer = std::make_unique<Printer>(id);
  if (!DictionaryToPrinter(pref, printer.get())) {
    LOG(WARNING) << "Failed to parse policy printer.";
    return nullptr;
  }

  printer->set_source(Printer::SRC_POLICY);

  const base::Value::Dict* ppd = pref.FindDict(kPpdResource);
  if (ppd) {
    Printer::PpdReference* ppd_reference = printer->mutable_ppd_reference();
    const std::string* make_and_model = ppd->FindString(kEffectiveModel);
    if (make_and_model)
      ppd_reference->effective_make_and_model = *make_and_model;
    std::optional<bool> autoconf = ppd->FindBool(kAutoconf);
    if (autoconf.has_value())
      ppd_reference->autoconf = *autoconf;
  }
  if (!printer->ppd_reference().autoconf &&
      printer->ppd_reference().effective_make_and_model.empty()) {
    // Either autoconf flag or make and model is mandatory.
    LOG(WARNING)
        << "Missing autoconf flag and model information for policy printer.";
    return nullptr;
  }
  if (printer->ppd_reference().autoconf &&
      !printer->ppd_reference().effective_make_and_model.empty()) {
    // PPD reference can't contain both autoconf and make and model.
    LOG(WARNING) << "Autoconf flag is set together with model information for "
                    "policy printer.";
    return nullptr;
  }

  return printer;
}

std::unique_ptr<Printer> ManagedPrinterToPrinter(
    const base::Value::Dict& managed_printer) {
  static auto LogRequiredFieldMissing = [](std::string_view field) {
    LOG(WARNING) << "Managed printer is missing required field: " << field;
  };

  const std::string* guid = managed_printer.FindString(kGuid);
  const std::string* display_name = managed_printer.FindString(kDisplayName);
  const std::string* uri = managed_printer.FindString(kUri);
  const base::Value::Dict* ppd_resource =
      managed_printer.FindDict(kPpdResource);
  const std::string* description = managed_printer.FindString(kDescription);
  if (!guid) {
    LogRequiredFieldMissing(kGuid);
    return nullptr;
  }
  if (!display_name) {
    LogRequiredFieldMissing(kDisplayName);
    return nullptr;
  }
  if (!uri) {
    LogRequiredFieldMissing(kUri);
    return nullptr;
  }
  if (!ppd_resource) {
    LogRequiredFieldMissing(kPpdResource);
    return nullptr;
  }

  auto printer = std::make_unique<Printer>(*guid);
  printer->set_source(Printer::SRC_POLICY);
  printer->set_display_name(*display_name);
  std::string set_uri_error_message;
  if (!printer->SetUri(*uri, &set_uri_error_message)) {
    LOG(WARNING) << base::StringPrintf(
        "Managed printer '%s' has invalid %s value: %s, error: %s",
        display_name->c_str(), kUri, uri->c_str(),
        set_uri_error_message.c_str());
    return nullptr;
  }
  if (!ValidateAndSetPpdReference(*ppd_resource, *printer)) {
    return nullptr;
  }
  if (description) {
    printer->set_description(*description);
  }
  return printer;
}

base::Value::Dict GetCupsPrinterInfo(const Printer& printer) {
  base::Value::Dict printer_info = CreateEmptyPrinterInfo();

  printer_info.Set("isManaged",
                   printer.source() == Printer::Source::SRC_POLICY);
  printer_info.Set("printerId", printer.id());
  printer_info.Set("printerName", printer.display_name());
  printer_info.Set("printerDescription", printer.description());
  printer_info.Set("printerMakeAndModel", printer.make_and_model());
  // NOTE: This assumes the the function IsIppEverywhere() simply returns
  // |printer.ppd_reference_.autoconf|. If the implementation of
  // IsIppEverywhere() changes this will need to be changed as well.
  printer_info.SetByDottedPath("printerPpdReference.autoconf",
                               printer.IsIppEverywhere());
  printer_info.Set("printerPPDPath",
                   printer.ppd_reference().user_supplied_ppd_url);
  printer_info.Set("printServerUri", printer.print_server_uri());
  printer_info.Set("printerStatus", printer.printer_status().ConvertToValue());

  if (!printer.HasUri()) {
    // Uri is invalid so we set default values.
    LOG(WARNING) << "Could not parse uri.  Defaulting values";
    printer_info.Set("printerAddress", "");
    printer_info.Set("printerQueue", "");
    printer_info.Set("printerProtocol", "ipp");  // IPP is our default protocol.
    return printer_info;
  }

  if (printer.IsUsbProtocol())
    printer_info.Set("ppdManufacturer", printer.usb_printer_manufacturer());
  printer_info.Set("printerProtocol", printer.uri().GetScheme());
  printer_info.Set("printerAddress", PrinterAddress(printer.uri()));
  std::string printer_queue = printer.uri().GetPathEncodedAsString();
  if (!printer_queue.empty())
    printer_queue = printer_queue.substr(1);  // removes the leading '/'
  if (!printer.uri().GetQueryEncodedAsString().empty())
    printer_queue += "?" + printer.uri().GetQueryEncodedAsString();
  printer_info.Set("printerQueue", printer_queue);

  return printer_info;
}

}  // namespace chromeos
