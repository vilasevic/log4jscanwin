// Main.cpp : This file contains the 'main' function. Program
// execution begins and ends there.
//

#include "stdafx.h"
#include "Utils.h"
#include "Reports.h"
#include "Scanner.h"
#include "MainScan.h"

#include "Version.info"


#define ARGX3(s1, s2, s3) \
  (!_wcsicmp(argv[i], s1) || !_wcsicmp(argv[i], s2) || !_wcsicmp(argv[i], s3))
#define ARG(S) ARGX3(L"-" #S, L"--" #S, L"/" #S)
#define ARGPARAMCOUNT(X) ((i + X) <= (argc - 1))


CCommandLineOptions cmdline_options;


int32_t PrintHelp(int32_t argc, wchar_t* argv[]) {
  int32_t rv = ERROR_SUCCESS;

  wprintf(L"/help\n");
  wprintf(L"  Displays this help page.\n");
  wprintf(L"/lowpriority\n");
  wprintf(L"  Lowers priority of Log4JScanner other processes have priority.\n");
  wprintf(L"/scan\n");
  wprintf(L"  Scan local drives for vulnerable JAR, WAR, EAR, PAR, ZIP files used by various Java applications.\n");
  wprintf(L"/scan_directory \"C:\\Some\\Path\"\n");
  wprintf(L"  Scan a specific directory for vulnerable JAR, WAR, EAR, PAR, ZIP files used by various Java applications.\n");
  wprintf(L"/scan_file \"C:\\Some\\Path\\Some.jar\"\n");
  wprintf(L"  Scan a specific file for supported CVE(s).\n");
  wprintf(L"/scaninclmountpoints\n");
  wprintf(L"  Scan local drives including mount points for vulnerable JAR, WAR, EAR, PAR, ZIP files used by various Java applications.\n");
  wprintf(L"/report\n");
  wprintf(L"  Generate a JSON report of possible detections of supported CVE(s).\n");
  wprintf(L"/report_pretty\n");
  wprintf(L"  Generate a human readable JSON report of possible detections of supported CVE(s).\n");
  wprintf(L"/report_sig\n");
  wprintf(L"  Generate a signature report of possible detections of supported CVE(s).\n");
  wprintf(L"\n");

  return rv;
}

int32_t ProcessCommandLineOptions(int32_t argc, wchar_t* argv[]) {
  int32_t rv = ERROR_SUCCESS;

  for (int32_t i = 1; i < argc; i++) {
    if (0) {
    } else if (ARG(scan)) {
      cmdline_options.scanLocalDrives = true;
    } else if (ARG(scan_file) && ARGPARAMCOUNT(1)) {
      cmdline_options.scanFile = true;
      cmdline_options.file = argv[i + 1];
    } else if (ARG(scan_directory) && ARGPARAMCOUNT(1)) {
      cmdline_options.scanDirectory = true;
      cmdline_options.directory = argv[i + 1];
    } else if (ARG(scaninclmountpoints)) {
      cmdline_options.scanLocalDrivesInclMountpoints = true;
    } else if (ARG(report)) {
      cmdline_options.no_logo = true;
      cmdline_options.report = true;
    } else if (ARG(report_pretty)) {
      cmdline_options.no_logo = true;
      cmdline_options.report = true;
      cmdline_options.reportPretty = true;
    } else if (ARG(report_sig)) {
      cmdline_options.no_logo = true;
      cmdline_options.report = true;
      cmdline_options.reportSig = true;
    } else if (ARG(nologo)) {
      cmdline_options.no_logo = true;
    } else if (ARG(lowpriority)) {
      cmdline_options.lowpriority = true;
    } else if (ARG(v) || ARG(verbose)) {
      cmdline_options.verbose = true;
    } else if (ARG(?) || ARG(h) || ARG(help)) {
      cmdline_options.help = true;
    }
  }

  //
  // Check to make sure the directory path is normalized
  //
  if (cmdline_options.scanDirectory) {
    if ((0 == cmdline_options.directory.substr(0, 1).compare(L"\"")) ||
        (0 == cmdline_options.directory.substr(0, 1).compare(L"'"))) {
      cmdline_options.directory.erase(0, 1);
    }
    if ((0 == cmdline_options.directory.substr(cmdline_options.directory.size() - 1, 1).compare(L"\"")) ||
        (0 == cmdline_options.directory.substr(cmdline_options.directory.size() - 1, 1).compare(L"'"))) {
      cmdline_options.directory.erase(cmdline_options.directory.size() - 1, 1);
    }
    if (0 != cmdline_options.directory.substr(cmdline_options.directory.size() - 1, 1).compare(L"\\")) {
      cmdline_options.directory += L"\\";
    }
  }

  return rv;
}

int32_t __cdecl wmain(int32_t argc, wchar_t* argv[]) {
  int32_t rv = ERROR_SUCCESS;

  SetUnhandledExceptionFilter(CatchUnhandledExceptionFilter);
  _setmode(_fileno(stdout), _O_U16TEXT);

#ifndef _WIN64
  using typeWow64DisableWow64FsRedirection = BOOL(WINAPI*)(PVOID OlValue);
  typeWow64DisableWow64FsRedirection Wow64DisableWow64FsRedirection;
  BOOL bIs64BitWindows = FALSE;
  PVOID pHandle;

  if (!IsWow64Process(GetCurrentProcess(), &bIs64BitWindows)) {
    wprintf(L"Failed to determine if process is running as WoW64.\n");
    goto END;
  }

  if (bIs64BitWindows) {
    Wow64DisableWow64FsRedirection =
        (typeWow64DisableWow64FsRedirection)GetProcAddress(
            GetModuleHandle(L"Kernel32.DLL"), "Wow64DisableWow64FsRedirection");

    if (Wow64DisableWow64FsRedirection) {
      Wow64DisableWow64FsRedirection(&pHandle);
    }
  }
#endif

  rv = ProcessCommandLineOptions(argc, argv);
  if (ERROR_SUCCESS != rv) {
    wprintf(L"Failed to process command line options.\n");
    goto END;
  }

  if (!cmdline_options.no_logo) {
    wprintf(L"Qualys Log4j Vulnerability Scanner %S\n", VERSION_STRING);
    wprintf(L"https://www.qualys.com/\n");
    wprintf(L"Supported CVE(s): CVE-2021-4104, CVE-2021-44228, CVE-2021-44832, CVE-2021-45046, CVE-2021-45105\n\n");
  }

  if (cmdline_options.help) {
    PrintHelp(argc, argv);
    goto END;
  }

  if (!cmdline_options.scanLocalDrives && !cmdline_options.scanNetworkDrives &&
      !cmdline_options.scanDirectory && !cmdline_options.scanFile && !cmdline_options.scanLocalDrivesInclMountpoints) {
    cmdline_options.scanLocalDrives = true;
  }

  if (cmdline_options.reportSig) {
    OpenStatusFile(GetSignatureStatusFilename());
  }
  
  if (cmdline_options.lowpriority) {
      if (!SetPriorityClass(GetCurrentProcess(), PROCESS_MODE_BACKGROUND_BEGIN))
      {
          wprintf(L"Failed to set process priority.\n");
      }
      else
      {
          if (cmdline_options.verbose) {
              wprintf(L"CPU and I/O priority lowered.\n");
          }
      }
  }

  repSummary.scanStart = time(0);

  if (cmdline_options.reportSig) {
    wchar_t buf[64] = {0};
    struct tm* tm = NULL;

    tm = localtime((time_t*)&repSummary.scanStart);
    wcsftime(buf, _countof(buf) - 1, L"%FT%T%z", tm);

    LogStatusMessage(L"Scan start time : %s\n", buf);
  }

  if (cmdline_options.scanLocalDrives) {
    if (!cmdline_options.no_logo) {
      wprintf(L"Scanning Local Drives...\n");
    }
    ScanLocalDrives(!cmdline_options.no_logo, cmdline_options.verbose);
  }
  
  if (cmdline_options.scanLocalDrivesInclMountpoints) {
     if (!cmdline_options.no_logo) {
       wprintf(L"Scanning Local Drives including Mountpoints...\n");
      }
      ScanLocalDrivesInclMountpoints(!cmdline_options.no_logo, cmdline_options.verbose);
  }
  
  if (cmdline_options.scanNetworkDrives) {
    if (!cmdline_options.no_logo) {
      wprintf(L"Scanning Network Drives...\n");
    }
    ScanNetworkDrives(!cmdline_options.no_logo, cmdline_options.verbose);
  }

  if (cmdline_options.scanDirectory) {
    if (!cmdline_options.no_logo) {
      wprintf(L"Scanning '%s'...\n", cmdline_options.directory.c_str());
    }
    ScanDirectory(!cmdline_options.no_logo, cmdline_options.verbose, cmdline_options.directory);
  }

  if (cmdline_options.scanFile) {
    if (!cmdline_options.no_logo) {
      wprintf(L"Scanning '%s'...\n", cmdline_options.file.c_str());
    }
    ScanFile(!cmdline_options.no_logo, cmdline_options.verbose, cmdline_options.file);
  }

  repSummary.scanEnd = time(0);
  
  if (cmdline_options.lowpriority) {
      if (!SetPriorityClass(GetCurrentProcess(), PROCESS_MODE_BACKGROUND_END))
      {
          wprintf(L"Failed to set process priority");
      }
  }
  
  if (cmdline_options.reportSig) {
    wchar_t buf[64] = {0};
    struct tm* tm = NULL;

    tm = localtime((time_t*)&repSummary.scanEnd);
    wcsftime(buf, _countof(buf) - 1, L"%FT%T%z", tm);

    LogStatusMessage(L"\nScan end time : %s\n", buf);
  }


  if (!cmdline_options.no_logo) {
    wchar_t buf[64] = {0};
    struct tm* tm = NULL;

    tm = localtime((time_t*)&repSummary.scanEnd);
    wcsftime(buf, _countof(buf) - 1, L"%FT%T%z", tm);

    wprintf(L"\nScan Summary:\n");
    wprintf(L"\tScan Date:\t\t %s\n", buf);
    wprintf(L"\tScan Duration:\t\t %lld Seconds\n", repSummary.scanEnd - repSummary.scanStart);
    wprintf(L"\tFiles Scanned:\t\t %lld\n", repSummary.scannedFiles);
    wprintf(L"\tDirectories Scanned:\t %lld\n", repSummary.scannedDirectories);
    wprintf(L"\tJAR(s) Scanned:\t\t %lld\n", repSummary.scannedJARs);
    wprintf(L"\tWAR(s) Scanned:\t\t %lld\n", repSummary.scannedWARs);
    wprintf(L"\tEAR(s) Scanned:\t\t %lld\n", repSummary.scannedEARs);
    wprintf(L"\tPAR(s) Scanned:\t\t %lld\n", repSummary.scannedPARs);
    wprintf(L"\tZIP(s) Scanned:\t\t %lld\n", repSummary.scannedZIPs);
    wprintf(L"\tVulnerabilities Found:\t %lld\n", repSummary.foundVunerabilities);
  }

  if (cmdline_options.report) {
    if (!cmdline_options.reportSig) {
      GenerateJSONReport(cmdline_options.reportPretty);
    } else {
      GenerateSignatureReport();
    }
  }

END:

  if (cmdline_options.reportSig) {
    if (error_array.empty()) {
      LogStatusMessage(L"Run status : Success\n");
      LogStatusMessage(L"Result file location : %s\n", GetSignatureReportFilename().c_str());
    } else {
      LogStatusMessage(L"Run status : Partially Successful\n");
      LogStatusMessage(L"Result file location : %s\n", GetSignatureReportFilename().c_str());

      LogStatusMessage(L"Errors :\n");
      for (const auto& e : error_array) {
        LogStatusMessage(L"%s\n", e.c_str());
      }
    }
  }

  CloseStatusFile();

  return rv;
}
