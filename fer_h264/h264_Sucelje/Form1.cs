﻿using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading;
using System.Windows.Forms;
using fer_h264;

namespace h264_Sucelje
{
    public partial class Form1 : Form
    {
        public Form1()
        {
            InitializeComponent();
            Kodiraj.Enabled = false;
        }

        public string Poruka
        {
            get { return label4.Text; }
            set { label4.Text = value; label4.Refresh(); }
        }

        private void Kodiraj_Click(object sender, EventArgs e)
        {
            Starter pokretac = new Starter();
            //Thread t = new Thread(new ThreadStart(h264_Sucelje.Program.Radi));
            String bla = openFileDialog1.FileName;
            pokretac.PostaviUlaz(ref bla);
            //MessageBox.Show(bla);
            //Bitmap bitmap = new Bitmap("d:\\slika.bmp");
            //pictureBox1.Image = bitmap;//.GetThumbnailImage(80, 44, null, System.IntPtr.Zero);
            //pictureBox1.Refresh();
            pokretac.PostaviInterval((int)FrameStart.Value, (int)FrameEnd.Value, (int)_qp.Value);
            _textStatus.Text = "Pokrećem koder";
            _textStatus.Refresh();
            //t.Start();
            pokretac.PokreniKoder();
            
            List<int> velicine = new List<int>();
            List<int> brojPSkip = new List<int>();
            List<int> broj16x16 = new List<int>();
            List<int> broj16x8 = new List<int>();
            List<int> broj8x16 = new List<int>();
            List<int> broj8x8 = new List<int>();
            List<int> vrijeme = new List<int>();
            int p1 = 0, p2 = 0, p3 = 0, p4 = 0, p5 = 0, p6 = 0, p7 = 0;
            pokretac.DohvatiKarakteristike(ref p1, ref p2, ref p3, ref p4, ref p5, ref p6, ref p7);
            brojPSkip.Add(p1); broj16x16.Add(p2); broj16x8.Add(p3); broj8x16.Add(p4); broj8x8.Add(p5);
            velicine.Add(p6); vrijeme.Add(p7);
            
            for (int i = (int)FrameStart.Value; i < (int)FrameEnd.Value; i++)
            {
                _textStatus.Text = String.Format("Završio #{0} frame", i.ToString());
                _textStatus.Refresh();
                pokretac.NastaviKoder();
                pokretac.DohvatiKarakteristike(ref p1, ref p2, ref p3, ref p4, ref p5, ref p6, ref p7);
                brojPSkip.Add(p1); broj16x16.Add(p2); broj16x8.Add(p3); broj8x16.Add(p4); broj8x8.Add(p5);
                velicine.Add(p6); vrijeme.Add(p7);
            }
            DataTable tablica = new DataTable("Podaci o frame-ovima");
            tablica.Columns.Add("Frame #", typeof(int));
            tablica.Columns.Add("Veličina (B)", typeof(int));
            tablica.Columns.Add("Potrebno vrijeme (ms)", typeof(int));
            tablica.Columns.Add("Broj P_Skip predikcija", typeof(int));
            tablica.Columns.Add("Broj P_L0_16x16 predikcija", typeof(int));
            tablica.Columns.Add("Broj P_L0_16x8 predikcija", typeof(int));
            tablica.Columns.Add("Broj P_L0_8x16 predikcija", typeof(int));
            tablica.Columns.Add("Broj P_L0_8x8 predikcija", typeof(int));

            List<Int32> redak = new List<int>(); redak.Add(0); redak.Add(0);
            for (int i = (int)FrameStart.Value; i <= (int)FrameEnd.Value; i++)
            {
                tablica.Rows.Add(redak as IConvertible);
                tablica.Rows[i - (int)FrameStart.Value]["Frame #"] = i;
                tablica.Rows[i - (int)FrameStart.Value]["Veličina (B)"] = velicine[i - (int)FrameStart.Value];
                tablica.Rows[i - (int)FrameStart.Value]["Potrebno vrijeme (ms)"] = vrijeme[i - (int)FrameStart.Value];
                tablica.Rows[i - (int)FrameStart.Value]["Broj P_Skip predikcija"] = brojPSkip[i - (int)FrameStart.Value];
                tablica.Rows[i - (int)FrameStart.Value]["Broj P_L0_16x16 predikcija"] = broj16x16[i - (int)FrameStart.Value];
                tablica.Rows[i - (int)FrameStart.Value]["Broj P_L0_16x8 predikcija"] = broj16x8[i - (int)FrameStart.Value];
                tablica.Rows[i - (int)FrameStart.Value]["Broj P_L0_8x16 predikcija"] = broj8x16[i - (int)FrameStart.Value];
                tablica.Rows[i - (int)FrameStart.Value]["Broj P_L0_8x8 predikcija"] = broj8x8[i - (int)FrameStart.Value];
            }
            _podaci.DataSource = tablica.DefaultView;
            //t.Abort();
            _textStatus.Text = "Koder završio s radom";
        }

        private void FrameStart_KeyUp(object sender, KeyEventArgs e)
        {
            FrameEnd.Value = FrameStart.Value;
        }

        private void button1_Click(object sender, EventArgs e)
        {
            Kodiraj.Enabled = false;
            openFileDialog1.Filter = "Y4M datoteke (*.y4m)|*.y4m";
            if (openFileDialog1.ShowDialog() == DialogResult.OK)
            {
                Kodiraj.Enabled = true;
            }
        }
    }
}
