using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using fer_h264;

namespace h264_Sucelje
{
    public partial class Form1 : Form
    {
        public Form1()
        {
            InitializeComponent();
            //MessageBox.Show("Odaberite radni direktorij");
            //if (!(openFileDialog1.ShowDialog() == DialogResult.OK))
            //{
            //    this.Close();
            //}
        }

        private void Kodiraj_Click(object sender, EventArgs e)
        {
            _textStatus.Text = "Koder radi";
            _textStatus.Refresh();
            Starter pokretac = new Starter();
            pokretac.PostaviInterval((int)FrameStart.Value, (int)FrameEnd.Value);
            pokretac.PokreniKoder();
            _textStatus.Text = "Koder završio s radom";
        }
    }
}
