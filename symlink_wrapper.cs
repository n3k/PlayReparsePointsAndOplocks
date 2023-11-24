using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace IOAClient
{
    internal class symlink_wrapper
    {
        public const int FILE_SHARE_NONE   = 0;
        public const int FILE_SHARE_READ   = 1;
        public const int FILE_SHARE_WRITE  = 2;
        public const int FILE_SHARE_DELETE = 4;

        const string MyDllPath = @"C:\Users\public\ioaclient\PlayReparsePointsAndOpLocks.dll";

        public delegate void UserCallback();


        [DllImport(MyDllPath, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern IntPtr create_oplock(string filename, uint shareMode, UserCallback opLockCallback);

        [DllImport(MyDllPath, CallingConvention = CallingConvention.Cdecl)]
        public static extern void wait_for_lock(IntPtr opLock, uint timeout);

        [DllImport(MyDllPath, CallingConvention = CallingConvention.Cdecl)]
        public static extern void delete_oplock(IntPtr opLock);

        [DllImport(MyDllPath, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern void create_pseudo_symlink(string src, string dst);

        [DllImport(MyDllPath, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern void delete_pseudo_symlink(string src, string dst);

        [DllImport(MyDllPath, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern void create_dos_device_symlink(string src, string dst);

        [DllImport(MyDllPath, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern void remove_dos_device_symlink(string src, string dst);

        [DllImport(MyDllPath, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        [return: MarshalAs(UnmanagedType.Bool)]
        public static extern bool create_mount_point(string srcDirectory, string dstDirectory);

        [DllImport(MyDllPath, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        [return: MarshalAs(UnmanagedType.Bool)]
        public static extern bool delete_mount_point(string path);
    }
}
