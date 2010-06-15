using System;
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
            Dekodiraj.Enabled = false;
        }

        int mozeKodirati;
        int mozeDekodirati;
        string kodirajUlaz;
        string kodirajIzlaz;
        string dekodirajUlaz;
        string dekodirajIzlaz;

        public string Poruka
        {
            get { return label4.Text; }
            set { label4.Text = value; label4.Refresh(); }
        }

        private void Kodiraj_Click(object sender, EventArgs e)
        {
            Starter pokretac = new Starter();
            
            // Postavljanje odabranog ulazne i izlazne datoteke.
            pokretac.PostaviUlazIzlaz(ref kodirajUlaz, ref kodirajIzlaz);
            int osnovnoInterPredvidanje = 0; 
            if (_osnovnoInter.Checked) osnovnoInterPredvidanje = 1;
            // Postavljanje parametara za koder.
            pokretac.PostaviParametre((int)FrameStart.Value, (int)FrameEnd.Value, (int)_qp.Value, osnovnoInterPredvidanje, (int)_velicinaProzora.Value, (int)_toleriranaGreska.Value);
            _textStatus.Text = "Pokrećem koder";
            _textStatus.Refresh();
            pokretac.PokreniKoder();
            
            // Definicija podataka koji se prikazuju u statistici po završetku kodera.
            List<int> velicine = new List<int>();
            List<int> brojPSkip = new List<int>();
            List<int> broj16x16 = new List<int>();
            List<int> broj16x8 = new List<int>();
            List<int> broj8x16 = new List<int>();
            List<int> broj8x8 = new List<int>();
            List<int> vrijeme = new List<int>();
            int p1 = 0, p2 = 0, p3 = 0, p4 = 0, p5 = 0, p6 = 0, p7 = 0;
            pokretac.DohvatiStatistiku(ref p1, ref p2, ref p3, ref p4, ref p5, ref p6, ref p7);
            brojPSkip.Add(p1); broj16x16.Add(p2); broj16x8.Add(p3); broj8x16.Add(p4); broj8x8.Add(p5);
            velicine.Add(p6); vrijeme.Add(p7);
            
            for (int i = (int)FrameStart.Value; i < (int)FrameEnd.Value; i++)
            {
                _textStatus.Text = String.Format("Završio #{0} frame", i.ToString());
                _textStatus.Refresh();
                pokretac.NastaviKoder();
                pokretac.DohvatiStatistiku(ref p1, ref p2, ref p3, ref p4, ref p5, ref p6, ref p7);
                brojPSkip.Add(p1); broj16x16.Add(p2); broj16x8.Add(p3); broj8x16.Add(p4); broj8x8.Add(p5);
                velicine.Add(p6); vrijeme.Add(p7);
            }

            //Inicijalizacija tablice statistike.
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

            // Prikaz statistike i postavljanje trenutnog statusa koda u završeno.
            _podaci.DataSource = tablica.DefaultView;
            _textStatus.Text = "Koder završio s radom";
        }

        private void FrameStart_KeyUp(object sender, KeyEventArgs e)
        {
            FrameEnd.Value = FrameStart.Value;
        }

        private void button1_Click(object sender, EventArgs e)
        {
            Kodiraj.Enabled = false;
            openFileDialog1.CheckFileExists = true;
            openFileDialog1.Filter = "Y4M datoteke (*.y4m)|*.y4m";
            if (openFileDialog1.ShowDialog() == DialogResult.OK)
            {
                kodirajUlaz = openFileDialog1.FileName;
                mozeKodirati |= 2;
                if (mozeKodirati == 3)
                {
                    Kodiraj.Enabled = true;
                }
            }
        }

        private void button5_Click(object sender, EventArgs e)
        {
            Dekodiraj.Enabled = false;
            openFileDialog1.CheckFileExists = false;
            openFileDialog1.Filter = "Y4M datoteke (*.y4m)|*.y4m";
            if (openFileDialog1.ShowDialog() == DialogResult.OK)
            {
                dekodirajIzlaz = openFileDialog1.FileName;
                mozeDekodirati |= 2;
                if (mozeDekodirati == 3)
                {
                    Dekodiraj.Enabled = true;
                }
            }  
        }

        private void button2_Click(object sender, EventArgs e)
        {
            Dekodiraj.Enabled = false;
            openFileDialog1.CheckFileExists = true;
            openFileDialog1.Filter = "264 datoteke (*.264)|*.264";
            if (openFileDialog1.ShowDialog() == DialogResult.OK)
            {
                dekodirajUlaz = openFileDialog1.FileName;
                mozeDekodirati |= 1;
                if (mozeDekodirati == 3)
                {
                    Dekodiraj.Enabled = true;
                }
            }  
        }

        private void button4_Click(object sender, EventArgs e)
        {
            Kodiraj.Enabled = false;
            openFileDialog1.CheckFileExists = false;
            openFileDialog1.Filter = "264 datoteke (*.264)|*.264";
            if (openFileDialog1.ShowDialog() == DialogResult.OK)
            {
                kodirajIzlaz = openFileDialog1.FileName;
                mozeKodirati |= 1;
                if (mozeKodirati == 3)
                {
                    Kodiraj.Enabled = true;
                }
            } 
        }

        private void Dekodiraj_Click(object sender, EventArgs e)
        {
            Starter pokretac = new Starter();
            pokretac.PostaviUlazIzlaz(ref dekodirajUlaz, ref dekodirajIzlaz);
            statusDekoder.Text = "Dekoder radi";
            pokretac.PokreniDekoder();
            statusDekoder.Text = "Dekoder završio s radom";
        }
    }
}
