﻿using System;
using System.Windows.Threading;
using System.Runtime.InteropServices;
using System.Windows.Media.Imaging;
using Microsoft.Win32;
using System.Security;
using System.IO;
using System.Windows;

namespace eIDViewer
{

    public partial class MainWindow : Window
    {  
        string ReadRegistryStringValue(string subkey, string valueName, object defaultValue)
        {
            const string userRoot = "HKEY_CURRENT_USER";
            const string localMachineRoot = "HKEY_LOCAL_MACHINE";
            string keyName = userRoot + "\\" + subkey;
            try
            {

                object readValue = Registry.GetValue(keyName, valueName, null);

                if (readValue == null)
                {
                    keyName = localMachineRoot + "\\" + subkey;
                    readValue = Registry.GetValue(keyName, valueName, null);
                }
                if (readValue == null)
                {
                    return "";
                }
                else
                {
                    return readValue as string;
                }
            }
            catch (SecurityException e)
            {
                theBackendData.logText += "ReadRegistryStringValue failed, no permission to read key " + keyName;
                return "";
            }
            catch (IOException e)
            {
                theBackendData.logText += "ReadRegistryStringValue failed, the key " + keyName + " was marked for deletion";
                return "";
            }
            catch (Exception e)
            {
                Console.WriteLine("{0} Exception caught.", e);
                return "";
            }
        }

        void WriteRegistryStringValue(string subkey, string valueName, object valueValue)
        {
            const string userRoot = "HKEY_CURRENT_USER";
            string keyName = userRoot + "\\" + subkey;
            try
            {
                Registry.SetValue(keyName, valueName, valueValue as string, RegistryValueKind.String);
            }

            catch (Exception e)
            {
               theBackendData.logText += "Exception caught when trying to write to registry key " + keyName;
               Console.WriteLine("{0} Exception caught.", e);
            }
        }

        public void StoreLanguage (string language)
        {
            WriteRegistryStringValue("SOFTWARE\\BEID\\general", "language", language);
        }

        public void GetLanguage()
        {
            string readValue = ReadRegistryStringValue("SOFTWARE\\BEID\\general", "language", null);
            if(readValue != null)
            {
                if( readValue.Equals("nl"))
                {
                    ChangeLocalization("nl-BE");
                }
                else if (readValue.Equals("fr"))
                {
                    ChangeLocalization("fr-BE");
                }
                else if (readValue.Equals("de"))
                {
                    ChangeLocalization("de-DE");
                }
                else if (readValue.Equals("en"))
                {
                    ChangeLocalization("en-US");
                }
                else
                {
                    theBackendData.logText += "unknow language identifier found in registry: " + readValue;
                }
            }
        }

    }

}
