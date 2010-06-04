namespace h264_Sucelje
{
    partial class Form1
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.FrameStart = new System.Windows.Forms.NumericUpDown();
            this.FrameEnd = new System.Windows.Forms.NumericUpDown();
            this.label1 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this._textStatus = new System.Windows.Forms.Label();
            this.Kodiraj = new System.Windows.Forms.Button();
            this.openFileDialog1 = new System.Windows.Forms.OpenFileDialog();
            ((System.ComponentModel.ISupportInitialize)(this.FrameStart)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.FrameEnd)).BeginInit();
            this.SuspendLayout();
            // 
            // FrameStart
            // 
            this.FrameStart.Location = new System.Drawing.Point(141, 44);
            this.FrameStart.Maximum = new decimal(new int[] {
            1000,
            0,
            0,
            0});
            this.FrameStart.Minimum = new decimal(new int[] {
            1,
            0,
            0,
            0});
            this.FrameStart.Name = "FrameStart";
            this.FrameStart.Size = new System.Drawing.Size(52, 20);
            this.FrameStart.TabIndex = 0;
            this.FrameStart.Value = new decimal(new int[] {
            1,
            0,
            0,
            0});
            // 
            // FrameEnd
            // 
            this.FrameEnd.Location = new System.Drawing.Point(141, 80);
            this.FrameEnd.Maximum = new decimal(new int[] {
            1000,
            0,
            0,
            0});
            this.FrameEnd.Minimum = new decimal(new int[] {
            1,
            0,
            0,
            0});
            this.FrameEnd.Name = "FrameEnd";
            this.FrameEnd.Size = new System.Drawing.Size(52, 20);
            this.FrameEnd.TabIndex = 1;
            this.FrameEnd.Value = new decimal(new int[] {
            1,
            0,
            0,
            0});
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(42, 46);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(68, 13);
            this.label1.TabIndex = 2;
            this.label1.Text = "Početni frejm";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(43, 82);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(67, 13);
            this.label2.TabIndex = 3;
            this.label2.Text = "Završni frejm";
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(42, 183);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(40, 13);
            this.label3.TabIndex = 4;
            this.label3.Text = "Status:";
            // 
            // _textStatus
            // 
            this._textStatus.AutoSize = true;
            this._textStatus.Location = new System.Drawing.Point(89, 183);
            this._textStatus.Name = "_textStatus";
            this._textStatus.Size = new System.Drawing.Size(46, 13);
            this._textStatus.TabIndex = 5;
            this._textStatus.Text = "Čekanje";
            // 
            // Kodiraj
            // 
            this.Kodiraj.Location = new System.Drawing.Point(106, 106);
            this.Kodiraj.Name = "Kodiraj";
            this.Kodiraj.Size = new System.Drawing.Size(87, 30);
            this.Kodiraj.TabIndex = 6;
            this.Kodiraj.Text = "Pokreni koder";
            this.Kodiraj.UseVisualStyleBackColor = true;
            this.Kodiraj.Click += new System.EventHandler(this.Kodiraj_Click);
            // 
            // openFileDialog1
            // 
            this.openFileDialog1.FileName = "openFileDialog1";
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(292, 206);
            this.Controls.Add(this.Kodiraj);
            this.Controls.Add(this._textStatus);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.FrameEnd);
            this.Controls.Add(this.FrameStart);
            this.Name = "Form1";
            this.Text = "H.264";
            ((System.ComponentModel.ISupportInitialize)(this.FrameStart)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.FrameEnd)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.NumericUpDown FrameStart;
        private System.Windows.Forms.NumericUpDown FrameEnd;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Label _textStatus;
        private System.Windows.Forms.Button Kodiraj;
        private System.Windows.Forms.OpenFileDialog openFileDialog1;

    }
}

