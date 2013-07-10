#include "StdAfx.h"
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <tchar.h>

#define SERVICE_NAME "MySql"

SERVICE_STATUS status;
TCHAR err[256];

LPTSTR GetLastErrorText( LPTSTR lpszBuf, DWORD dwSize )
{
    DWORD dwRet;
    LPTSTR lpszTemp = NULL;
    dwRet = FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |FORMAT_MESSAGE_ARGUMENT_ARRAY,
                           NULL,
                           GetLastError(),
                           LANG_NEUTRAL,
                           (LPTSTR)&lpszTemp,
                           0,
                           NULL );

    // supplied buffer is not long enough

    if ( !dwRet || ( (long)dwSize < (long)dwRet+14 ) )
        lpszBuf[0] = TEXT('\0');
    else
    {
        lpszTemp[lstrlen(lpszTemp)-2] = TEXT('\0');  //remove cr and newline character
        _stprintf( lpszBuf, TEXT("%s (0x%x)"), lpszTemp, GetLastError() );
    }
    if ( lpszTemp )
        LocalFree((HLOCAL) lpszTemp );
    return lpszBuf;
}

void service_command(bool start)
{
  SC_HANDLE service;
  SC_HANDLE ctrl_manager;

  ctrl_manager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

  if (ctrl_manager)
  {
    service = OpenService(ctrl_manager, TEXT(SERVICE_NAME), SERVICE_ALL_ACCESS);
    if (service)
    {
      if (start)
      {
        if(!ControlService(service, SERVICE_CONTROL_INTERROGATE, &status))
        {
          if(GetLastError()==ERROR_SERVICE_NOT_ACTIVE)
            if (!StartService(service, 0,NULL))
              printf ("Error starting the service - %s\n", GetLastErrorText(err, 256));
            else
              printf("Service started successfully!!%s\n", GetLastErrorText(err, 256));
          else
            printf ("ControlService failed - %s\n", GetLastErrorText(err, 256));
        }
        printf ("Some error\n");
      }
      else
      {
        if (!ControlService(service, SERVICE_CONTROL_STOP, &status))
          printf ("Error stoping service...");
        else
        {
          printf ("Stopping service .");

           while (!ControlService(service, SERVICE_CONTROL_INTERROGATE, &status))
             if(GetLastError()==ERROR_SERVICE_NOT_ACTIVE)
             {
               printf ("service stopped..\n");
               break;
             }
             else
               printf (".");
          //loop until service is stopped.
        }
      }
      CloseServiceHandle(service);
    }
    else
      printf ("OpenService failed - %s\n", GetLastErrorText(err, 256));
    CloseServiceHandle(ctrl_manager);
  }
  else
    printf ("OpenService failed - %s\n", GetLastErrorText(err, 256));
}

void main(int argc, char **argv)
{
   if ( argc > 1 && (*argv[1] == '-' || *argv[1] == '/') )
    {
      if ( stricmp("start", argv[1]+1) == 0)
        service_command(true);
      else
        if (stricmp("stop", argv[1]+1) == 0)
          service_command(false);
        else
          goto usage;
      return;
   }
usage:
  printf( "service -start     to remove the service\n");
  printf( "service -stop      to remove the service\n");
}
