using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

using K4os.Compression.LZ4;

namespace LZ4_Reader
{
    public partial class Form1 : Form
    {
        public Form1()
        {
            InitializeComponent();
        }
        public int offset;
        unsafe public struct ContentStruct
        {
            //[MarshalAs(UnmanagedType.ByValArray, SizeConst = 1036)]
            public uint fileLength;
            public uint offset;
            public uint size;
        }

        List<ContentStruct> fileData = new List<ContentStruct>();

        unsafe private void Form1_Load(object sender, EventArgs e)
        {
            //offset = 4;
            //MessageBox.Show(sizeof(MyStruct).ToString());
        }

        static string getString(char[] arr)
        {
            return new string(arr);
        }

        unsafe private void button1_Click(object sender, EventArgs e)
        {
            string currentPath = Environment.CurrentDirectory;

            OpenFileDialog openFileDialog = new OpenFileDialog();
            openFileDialog.InitialDirectory = currentPath;
            openFileDialog.Filter = "All files (*.*)|*.*";
            openFileDialog.FilterIndex = 2;
            openFileDialog.RestoreDirectory = true;

            if (openFileDialog.ShowDialog() == DialogResult.OK)
            {
                listView1.Items.Clear();
                string filePath = openFileDialog.FileName;

                FileInfo fileInfo = new FileInfo(filePath);

                long fileSizeInBytes = fileInfo.Length;

                int files = 0;
                offset = 4;
                while (offset < fileSizeInBytes)
                {
                    using (BinaryReader reader = new BinaryReader(File.Open(filePath, FileMode.Open)))
                    {
                        string offsetText = "Starts from " + offset.ToString();
                        ContentStruct entry = new ContentStruct();
                        reader.BaseStream.Seek(offset, SeekOrigin.Begin);

                        byte[] buffer = new byte[12];
                        reader.BaseStream.Read(buffer, 0, buffer.Length);


                        offset += 12;
                        reader.BaseStream.Seek(offset, SeekOrigin.Begin);
                        byte[] charArray = reader.ReadBytes(1024);
                        offset += 1024;

                        GCHandle handle = GCHandle.Alloc(buffer, GCHandleType.Pinned);
                        entry = (ContentStruct)Marshal.PtrToStructure(handle.AddrOfPinnedObject(), typeof(ContentStruct));
                        handle.Free();

                        offset += (int)entry.size;

                        reader.BaseStream.Seek(offset, SeekOrigin.Begin);
                        byte[] encContent = reader.ReadBytes((int)entry.size);

                        files++;
                        /*if (entry.size != entry.fileLength) //TODO: Add decompression function
                        {
                            var source = encContent;
                            var target = new byte[entry.fileLength];
                            var decoded = LZ4Codec.Decode(
                                source, 0, source.Length,
                                target, 0, target.Length);
                            MessageBox.Show(Encoding.UTF8.GetString(target));
                            //var ee = new byte[encContent.Length];
                            //LZ4Codec.Decode(encContent, (int)entry.size, dest, (int)entry.fileLength);
                        }*/
                        offsetText += " until offset " + (offset).ToString();
                        ListViewItem itm = new ListViewItem(Encoding.UTF8.GetString(charArray));
                        itm.SubItems.Add(offsetText);
                        itm.SubItems.Add(entry.size.ToString());
                        itm.SubItems.Add(entry.fileLength.ToString());
                        listView1.Items.Add(itm);

                        fileData.Add(entry);
                    }
                }
            }
        }

        private void clickSelectedFile(object sender, EventArgs e)
        {
            //TODO: Decompile file by clicking on it at its path
            //MessageBox.Show("click");
        }
    }
}
