using System;
using System.Xml.Serialization;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using MonoCSharp;

namespace WinFormTest
{
    public class ClassA
    {
        public string name;
        public ClassB obj;
    }
    public class ClassB
    {
        public float value;
    }
    public partial class Form1 : Form
    {
        public Form1()
        {
            InitializeComponent();
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            ClassB objb = new ClassB(); objb.value = 233.3f;
            ClassA obja = new ClassA(); obja.obj = objb; obja.name = "zqj";
            XmlSerializer serializer = new XmlSerializer(typeof(ClassA));
            System.IO.StringWriter writer = new System.IO.StringWriter();
            serializer.Serialize(writer, obja);
            System.IO.File.WriteAllText(Environment.GetFolderPath(Environment.SpecialFolder.Desktop) + "\\test.txt", writer.ToString());
            MessageBox.Show("Done");
        }
    }
}
