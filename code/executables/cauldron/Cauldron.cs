using System;
using System.Collections.Generic;
using System.Drawing;
using System.Drawing.Imaging;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace cauldron
{
  public class Cauldron
  {
    private Bitmap m_Image;

    public Cauldron()
    {
      m_Image = new Bitmap(1280, 960, System.Drawing.Imaging.PixelFormat.Format32bppArgb);

      for (int i = 0; i < 1280; ++i)
        for (int j = 0; j < 960; ++j)
        {
          m_Image.SetPixel(i, j, Color.Black);
        }
    }

    public Image Screen
    {
      get
      {
        return m_Image;
      }
    }


    public Bitmap GetImage()
    {
      UInt32 size = 1280 * 960 * 4;
      byte[] test = new byte[size];
      test[3] = 4;
      FetchCurrentFrame_PInvoke(test, size);
      return CopyDataToBitmap(test);
    }

    private Bitmap CopyDataToBitmap(byte[] data)
    {
      //Here create the Bitmap to the know height, width and format
      Bitmap bmp = new Bitmap(1280, 960, PixelFormat.Format32bppArgb);

      //Create a BitmapData and Lock all pixels to be written
      BitmapData bmpData = bmp.LockBits(
      new Rectangle(0, 0, bmp.Width, bmp.Height),
      ImageLockMode.WriteOnly, bmp.PixelFormat);

      //Copy the data from the byte array into BitmapData.Scan0
      Marshal.Copy(data, 0, bmpData.Scan0, data.Length);
      //Unlock the pixels
      bmp.UnlockBits(bmpData);
      //Return the bitmap
      return bmp;
    }

    [DllImport("cauldron_native", CallingConvention = CallingConvention.Cdecl)]
    private static extern void FetchCurrentFrame_PInvoke(byte[] outImg, UInt32 size);

  }

}
