#include "stdafx.h"
#include<Kinect.h>
#include<opencv/cv.hpp>
#include <opencv2/imgproc.hpp>  
#include <opencv2/highgui.hpp>  
#include <iostream>
#include<string>
#include<sstream>
#include<fstream>
#pragma comment ( lib, "kinect20.lib" ) 

using   namespace   std;
using   namespace   cv;
void judege();
void mythreshold(Mat img, int dis);
void u16threshold(Mat img, int dis);
void getdepth(Mat sourceimg, Mat binary, Point center, int difx, int dify, int r, int type);
void mgetdepth(Mat sourceimg, Mat binary, Point center, int difx, int dify, int r, int type);
void hitdepth(Mat sourceimg, Mat binary, Point center, int difx, int dify, int r);
ushort singleBdata[200][200] = {0};
ushort ringdata[200] = {0};
ushort Bdata[200][4] = {0};
ushort Rdata[1][4] = {0};
int ord = 0;
bool bnd = false;

int main(void)
{
	// 获取感应器
	IKinectSensor* pSensor = nullptr;
	GetDefaultKinectSensor(&pSensor);

	// 打开感应器
	pSensor->Open();

	//  取得深度数据
	IDepthFrameSource* pFrameSource = nullptr;
	pSensor->get_DepthFrameSource(&pFrameSource);

	// 取得深度数据的描述信息（宽、高）
	int        iWidth = 0;
	int        iHeight = 0;
	int		   bgt = 0;
	IFrameDescription* pFrameDescription = nullptr;
	pFrameSource->get_FrameDescription(&pFrameDescription);
	pFrameDescription->get_Width(&iWidth);
	pFrameDescription->get_Height(&iHeight);
	pFrameDescription->Release();
	pFrameDescription = nullptr;

	//  取得感应器的“可靠深度”的最大、最小距离
	UINT16 uDepthMin = 0, uDepthMax = 0;
	pFrameSource->get_DepthMinReliableDistance(&uDepthMin);
	pFrameSource->get_DepthMaxReliableDistance(&uDepthMax);

	// 建立图像矩阵，mDepthImg用来存储16位的图像数据，直接用来显示会接近全黑
	//不方便观察，用mImg8bit转换成8位再显示
	cv::Mat dpImg(iHeight, iWidth, CV_16UC1);
	cv::Mat dpthres(iHeight, iWidth, CV_16UC1);
	cv::Mat dpthrescol(iHeight, iWidth, CV_16UC3);
	cv::Mat mImg8bit(iHeight, iWidth, CV_8UC1);
	cv::Mat dpthres8(iHeight, iWidth, CV_8UC1);
	cv::Mat bgdthres(iHeight, iWidth, CV_8UC1);
	cv::Mat bgdbinary(iHeight, iWidth, CV_8UC1);
	cv::Mat diffbinary(iHeight, iWidth, CV_8UC1);
	Mat bgd(iHeight, iWidth, CV_8UC1);
	Mat roithres(iHeight, iWidth, CV_8UC1);
	Mat diff(iHeight, iWidth, CV_8UC1);
	Mat cdiff(iHeight, iWidth, CV_8UC1);
	Mat cdiffthres(iHeight, iWidth, CV_8UC1);

	//  打开深度数据阅读器
	IDepthFrameReader* pFrameReader = nullptr;
	pFrameSource->OpenReader(&pFrameReader);


	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;

	string res;
	string tag = "D:\\Bricks\\Cprj\\t\\cdiff";
	string tag1 = "D:\\Bricks\\Cprj\\t\\diff";
	string  type = ".jpg";
	stringstream ss;


	int i = 0;
	
	
	
	int BB = 0;
	int CC[150] = { 0 };
	double clcx = 0, clcy = 0, clcr = 0;
	double mj;


    size_t I = 0;
    int ct = 0;
	int rtx1 =130;
	int rty1 =140;
	int rtx2 =400-rtx1;
	int rty2 = 380-rty1;
	UINT16 depth=0;
	UINT16 Bdepth=0;
	Rect rect(rtx1, rty1,rtx2, rty2);
	Point center;
	Point Bcenter;
	int radius;
	int Bradius;


	int bgdcct = 0;
	vector<Vec3f> circles;
	// 主循环
Re: ct = 0;
	clcx = 0; clcy = 0; clcr = 0;
	bgdcct=0;





	while (1)
	{
		//  取得最新数据
		IDepthFrame* pFrame = nullptr;
	
		
		if (pFrameReader->AcquireLatestFrame(&pFrame) == S_OK)
		{
		
		
			ct++;
			//  把数据存入16位图像矩阵中
			pFrame->CopyFrameDataToArray(iWidth * iHeight,
				reinterpret_cast<UINT16*>(dpImg.data));//制转换数据类型
			pFrame->Release();

				//  把16位转换成8位


				//imshow("u16depthh0", dpImg);
				//imwrite("d://u16depth.jpg", dpImg);



				//复制避免破坏原始数据，进行空间截取
				dpthres = dpImg.clone();				
				u16threshold(dpthres,1000);


				dpthres.convertTo(dpthres8, CV_8U, 255.0f / uDepthMax);//转换为8位使识别明显
				imwrite("d://u16depthres.jpg", dpthres8);
				//imshow("depthres8", dpthres8);






				if (ct >= 4 && ct <= 8)
				{  
					//环心识别
					bgdthres = dpthres8.clone();
					bgdthres = bgdthres(rect);
					HoughCircles(bgdthres, circles, CV_HOUGH_GRADIENT, 1, 100, 100, 28, 35, 50);
			imshow("cut",bgdthres );

					for (I=0; I < circles.size(); I++)
					{
						clcx = clcx +circles[I][0];	clcy = clcy +circles[I][1];	clcr = clcr + circles[I][2];
						cout<<"find" << "x" << circles[I][0] << "y" << circles[I][1] << "r" << circles[I][2] << endl;
						bgdcct++;
						
					}
					

					 //环心定位，差分背景选定    
					    if (ct == 8)
						{
							bgd = bgdthres;
							 center.x = cvRound(clcx / bgdcct) + rtx1;
								center.y=  cvRound(clcy/bgdcct)+rty1;
							 radius = cvRound(clcr/bgdcct);
							 cout << "x" << center.x << "y" << center.y << "r" <<radius<< endl;
							if (radius<41)//(center.x <257) || (center.x >260) || (center.y <135) || (center.y > 139|| radius<38)
							  goto Re;


							 Rdata[0][0] = center.x;
							 Rdata[0][1] = center.y;
							 Rdata[0][3] = radius;

							 cvtColor(dpthres8, dpthrescol,COLOR_GRAY2RGB);
							
							 circle(dpthrescol,center,radius,255,1);
							 cout << cvRound(center.x)<<endl;
							 cout << cvRound(center.y)<<endl;
						
							 cout<<radius<<endl;

							 threshold(bgd, bgdbinary, 10, 255.0, CV_THRESH_BINARY);
							// imwrite("d://bgdcircle.jpg", dpthrescol);
							 imshow("depthImgbgdcircles", dpthrescol);
							 //imshow("bgd", bgd);
							 //imshow("bgdbinary", bgdbinary);
							 getdepth(dpthres, bgdbinary, center, rtx1, rty1, radius, 1);
										 }

											}


		 if (ct>8)
			{
			    cvtColor(dpthres8, dpthrescol, COLOR_GRAY2RGB);
				roithres = dpthres8.clone();
				roithres = roithres(rect);
				HoughCircles(roithres, circles, CV_HOUGH_GRADIENT, 1, 100, 100, 28, 35, 50);
				if (circles.size() == 0)
					cout << "obscale" << endl;
				diff = bgd.clone();
				cdiff = dpthres8.clone();
			//	imshow("roithres",roithres);
				
			//imshow("bgd", bgd);
				

				absdiff( roithres,bgd,diff);


				medianBlur(diff, diff, 9);
				threshold(diff, diffbinary, 10, 255.0, CV_THRESH_BINARY);
				//imshow("diff", diffbinary);
		/*		ss << tag1;
				ss << ct;
				ss << type;
				ss >> res;
				imwrite(res, diffbinary);
				cout << res;
				ss.clear();*/

				findContours(diffbinary, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));
				vector<Point2f>centerr(contours.size());
				vector<float>radiusr(contours.size());
				int contoursct = 0;
				for (int t = 0; t < contours.size(); t++)
				{
					mj = contourArea(contours[t]), arcLength(contours[t], true);
					if (mj >40 && mj < 2400)
					{
						//contoursct++;
						//if (contoursct>1)
						//
						//{
						//	if (contoursct>1)
						//	{
						//		//cout << "bounced" << endl;
						//		bnd = true;
						//		break;
						//	}
						//	bnd = true;
						//	break;
						//};
						minEnclosingCircle(contours[t], centerr[t], radiusr[t]);
						
						Bcenter.x = (int)centerr[t].x+rtx1;  Bcenter.y = (int)centerr[t].y+rty1; Bradius = (int)radiusr[t];
						circle(cdiff,Bcenter,Bradius,255,1);
				//imshow("cdiff", cdiff);
				/*		ss <<tag;
						ss << ct;
						ss << type;
						ss >> res;*/
				//imwrite(res, cdiff);
						//cout << res;
				/*		ss.clear();

						ss << tag1;
						ss << ct;
						ss << type;
						ss >> res;*/
				//imwrite(res, diffbinary);
						//cout << res;
				/*		ss.clear();*/
						int br = int(Bradius/2);
						mgetdepth(dpthres, diffbinary, Bcenter, rtx1, rty1,br,0);
						

					 }}


		/*			   if (contoursct>2)
						{
							cout<< "bounced"<<endl;
							bnd = true;
							break;
						}*/
		
		     }

		}

	
		
			
			 

		if (cv::waitKey(30) == VK_ESCAPE) {
			
			cout << "data get done"<<endl;
			std::ofstream out("out.txt", std::ios::out | std::ios::app);
			if (out.is_open())
			{
				int i;
				out.seekp(0, ios::beg);
				out << "R  " <<Rdata[0][0]<<" " << Rdata[0][1] << " " << Rdata[0][2] << " " << Rdata[0][3]<<endl;
				for(i=0;i<ord;i++)
				out << "B"<<i<<" " << Bdata[i][0] << " " <<Bdata[i][1] << " " << Bdata[i][2] << " " << Bdata[i][3] << endl;
				out.close();
			}
			else cout<<"openfailed"<<endl;
			break;
		}
		
	}


	judege(); 
		cout << "judged";
	//if (bnd == false)
	// {
	//
	//   }
	//else
	//{
	//	std::ofstream out("out.txt", std::ios::out | std::ios::app);
	//	if (out.is_open())
	//	{
	//		int i;
	//		out.seekp(0, ios::beg);
	//		out << "bounced" << endl;

	//		out.close();
	//	}
	//	else cout << "openfailed" << endl;
	//	cout << "bounced1";
	//    };

	
			if (cv::waitKey(0) == VK_ESCAPE)
				;
		
	// 3b. 释放变量pFrameReader
	pFrameReader->Release();
	pFrameReader = nullptr;

	// 2d.释放变量pFramesSource
	pFrameSource->Release();
	pFrameSource = nullptr;

	// 1c.关闭感应器
	pSensor->Close();

	// 1d.释放感应器
	pSensor->Release();
	pSensor = nullptr;

	return 0;
}

















void  mythreshold(Mat img, int dis)
{
	threshold(img, img, 168 - dis, 255, THRESH_TOZERO);
	threshold(img, img, 228 + dis, 255, THRESH_TOZERO_INV);
}
void  u16threshold(Mat img, int dis)
{
	threshold(img, img, 1200- dis, 255, THRESH_TOZERO);
	threshold(img, img, 4200 + dis, 255, THRESH_TOZERO_INV);
}
void getdepth(Mat sourceimg, Mat binary, Point center, int difx, int dify, int r, int type)
{
	cout << "in";
	int i = 0, j = 0;
	ushort gct = 0;
	UINT32 sum = 0;
	UINT16 dp = 0;
	if (type == 1)
	{
		
	/*	if (sourceimg.isContinuous() == 1)
			cout<<"sourcecontinuous";
		if (binary.isContinuous() == 1)
			cout<<"binarycontinuous";*/
		for (i = center.y - dify + 10; i <= center.y - dify + r; i++)
		{
			for (j = center.x - difx - r; j <= center.x - difx + r; j++)

			{

				UINT8* data = binary.ptr<UINT8>(i);
				UINT16* sdata = sourceimg.ptr<UINT16>(i);
				if (data[j] != 0 && sdata[j] != 0)
				{
					dp = sdata[j];
					ringdata[gct] = sdata[j];
					//cout << "sdt[" << gct << "]" << ringdata[gct]<<endl;
					sum += dp;
					//cout << "dp" << dp << endl;
					//cout << "gct" << gct << endl;
					gct++;
					if (gct > 160)
					{
						Rdata[0][2] = ushort(sum / gct);

						break;
					}



				}

			}
			if (gct > 160)
			{
				
				break;
	          	}
		
		
		}
		if(Rdata[0][2]==0)
				Rdata[0][2] = ushort(sum / gct);
		cout << "breaked";
		cout << "gctfinal" << gct<< endl;
		cout << "sum/gct" << (sum / gct )<< endl;
		cout << "dep" << Rdata[0][2] << endl;
		cout << "rx" << Rdata[0][0] << endl;
		cout << "ry" << Rdata[0][1] << endl;
		cout << "r" << Rdata[0][3] << endl;
		
	}



	if (type == 0)
	{
		
		Bdata[ord][0]=center.x;
		Bdata[ord][1]=center.y;	
		Bdata[ord][3]=2*r;	
	/*	if (sourceimg.isContinuous() == 1)
				cout<<"sourcecontinuous";
				if (binary.isContinuous() == 1)
				cout<<"binarycontinuous"<<endl;*/
		for (i = center.y - dify ; i <= center.y - dify + r; i++)
		{
			for (j = center.x - difx; j <= center.x - difx + r; j++)

			{
				
				UINT8* data = binary.ptr<UINT8>(i);
				UINT16* sdata = sourceimg.ptr<UINT16>(i);
				if (data[j] != 0 && sdata[j] != 0)
				{
					dp = sdata[j];
					singleBdata[ord][gct] = sdata[j];
					//cout << "sdt[" << ord << "]" <<"["<<gct<<"]"<< singleBdata[ord][gct] << endl;
					sum += dp;
					//cout << "dp" << dp << endl;
					//cout<<"gct0"<<gct<<endl;
					gct++;
					if (gct > 50)
					{cout << "sum" << sum<< endl;
					if (gct != 0)
						Bdata[ord][2] = ushort(sum/gct);

						break;
					}



				}
			/*	else
				{
					cout<<"datai"<< data[j]<<"sdata"<< sdata[j]<<endl;
				}*/

			}
			if (gct >50)
			{
				if(Bdata[ord][2]==0)
					if(gct!=0)
					Bdata[ord][2] = ushort(sum/gct);

				break;
			}


		}
		cout << "sum" << sum<< endl;
		if ((Bdata[ord][2] == 0)&&gct!=0)
		   Bdata[ord][2] = ushort(sum/gct);
				
			cout << "Bdata[" << ord << "]" << "[" << 0 << "]" << Bdata[ord][0] << endl;
			cout << "Bdata[" << ord << "]" << "[" << 1 << "]" << Bdata[ord][1] << endl;
			cout << "Bdata[" << ord << "]" << "[" << 2 << "]" << Bdata[ord][2] << endl;
			cout << "Bdata[" << ord << "]" << "[" << 3 << "]" << Bdata[ord][3] << endl;
			if (Bdata[ord][2] != 0)
				    ord++;

	     }
	


}
//[x,y,dp,r]
void judege()
{
	int k1=0,k2=ord-1,i,j,dis0=20000,dis,bounce=1;
	for (i = ord-1; i >=0; i--)
		if (Bdata[i][2] >Rdata[0][2])
			{
			bounce = 0;
			cout << "not bounced" << endl;
			for (j = ord - 1; j >= 0; j--)
			{
				dis = (Bdata[j][2] - Rdata[0][2]);
				//cout << "dis      " << dis;
				if ( dis>0)
				{
					
					if (dis < dis0)
					{
						dis0 = dis;
						k2 = j;
						cout << "dis0      " << dis0;
					}

				}
				else
				{
					k1 = j;
				};
				if (k1 != 0)
				{
					ushort x,y;
					if ((Bdata[k2][2] - Rdata[0][2]) > (Rdata[0][2] - Bdata[k1][2]))
					{
						x = Bdata[k1][0];
						y = Bdata[k1][1];
					}
					else 
					{
						x = Bdata[k2][0];
						y = Bdata[k2][1];
					};
					cout << "k1  " << k1 << "    k2  " << k2 << endl;
					if ((x >= Rdata[0][0] - Rdata[0][3] - Bdata[k1][3] ) && (x <= Rdata[0][0] + Rdata[0][3] - Bdata[k1][3] ) && (y >= Rdata[0][1] - Rdata[0][3] - Bdata[k1][3]) && (y <= Rdata[0][1] + Rdata[0][3] - Bdata[k1][3] ))
					{
						std::ofstream out("out.txt", std::ios::out | std::ios::app);
						if (out.is_open())
						{
							int i;
							out.seekp(0, ios::beg);
							out <<"x" <<x<<"  y"<<y<<"  success" << endl;

							out.close();
						}
						else cout << "openfailed" << endl;
						cout << "sucess";


				/*		ushort x = Bdata[k1][0] + (Rdata[0][2] - Bdata[k1][2]) / (Bdata[k2][2] - Bdata[k1][2])*(Bdata[k2][0] - Bdata[k1][0]);
						ushort y = Bdata[k1][1] + (Rdata[0][2] - Bdata[k1][2]) / (Bdata[k2][2] - Bdata[k1][2])*(Bdata[k2][1] - Bdata[k1][1]);
						cout << "k1  " << k1 << "    k2  " << k2 << endl;
						if ((x >= Rdata[0][0] - Rdata[0][3] - (Bdata[k1][3] + Bdata[k2][3]) / 2) && (x <= Rdata[0][0] + Rdata[0][3] - (Bdata[k1][3] + Bdata[k2][3]) / 2) && (y >= Rdata[0][1] - Rdata[0][3] - (Bdata[k1][3] + Bdata[k2][3]) / 2) && (y <= Rdata[0][1] + Rdata[0][3] - (Bdata[k1][3] + Bdata[k2][3]) / 2))
						{
							std::ofstream out("out.txt", std::ios::out | std::ios::app);
							if (out.is_open())
							{
								int i;
								out.seekp(0, ios::beg);
								out << "x" << x << "  y" << y << "  success" << endl;

								out.close();
							}
							else cout << "openfailed" << endl;
							cout << "sucess";*/







					}
					else 
					{
						std::ofstream out("out.txt", std::ios::out | std::ios::app);
						if (out.is_open())
						{
							int i;
							out.seekp(0, ios::beg);
							out << "x" << x << "  y" << y << "  faild" << endl;

							out.close();
						}
						else cout << "openfailed" << endl;
						cout << "faild";
			              	};
						
						break;
				}
			}
			break;
		         }

	if (bounce == 1)
	{ 
		std::ofstream out("out.txt", std::ios::out | std::ios::app);
		if (out.is_open())
		{
			int i;
			out.seekp(0, ios::beg);
			out << "bounced" << endl;

			out.close();
		}
		else cout << "openfailed" << endl;
		cout<< "bounced back"; };
		

 }



void mgetdepth(Mat sourceimg, Mat binary, Point center, int difx, int dify, int r, int type)
{
	cout << "in";
	int i = 0, j = 0;
	ushort gct = 0;
	ushort temp = 0;
	ushort medi = 0;
	UINT32 sum = 0;
	UINT16 dp = 0;
	if (type == 1)
	{

		/*	if (sourceimg.isContinuous() == 1)
		cout<<"sourcecontinuous";
		if (binary.isContinuous() == 1)
		cout<<"binarycontinuous";*/
		for (i = center.y - dify + 10; i <= center.y - dify + r; i++)
		{
			for (j = center.x - difx - r; j <= center.x - difx + r; j++)

			{

				UINT8* data = binary.ptr<UINT8>(i);
				UINT16* sdata = sourceimg.ptr<UINT16>(i);
				if (data[j] != 0 && sdata[j] != 0)
				{
					dp = sdata[j];
					ringdata[gct] = sdata[j];
					//cout << "sdt[" << gct << "]" << ringdata[gct]<<endl;
					sum += dp;
					//cout << "dp" << dp << endl;
					//cout << "gct" << gct << endl;
					gct++;
					if (gct > 160)
					{
						Rdata[0][2] = ushort(sum / gct);

						break;
					}



				}

			}
			if (gct > 160)
			{

				break;
			}


		}
		if (Rdata[0][2] == 0)
			Rdata[0][2] = ushort(sum / gct);
		//cout << "breaked";
		//cout << "gctfinal" << gct << endl;
		//cout << "sum/gct" << (sum / gct) << endl;
		cout << "dep" << Rdata[0][2] << endl;
		cout << "rx" << Rdata[0][0] << endl;
		cout << "ry" << Rdata[0][1] << endl;
		cout << "r" << Rdata[0][3] << endl;

	}



	if (type == 0)
	{

		Bdata[ord][0] = center.x;
		Bdata[ord][1] = center.y;
		Bdata[ord][3] = 2 * r;
		/*	if (sourceimg.isContinuous() == 1)
		cout<<"sourcecontinuous";
		if (binary.isContinuous() == 1)
		cout<<"binarycontinuous"<<endl;*/
		for (i = center.y - dify; i <= center.y - dify + r; i++)
		{
			for (j = center.x - difx; j <= center.x - difx + r; j++)

			{

				UINT8* data = binary.ptr<UINT8>(i);
				UINT16* sdata = sourceimg.ptr<UINT16>(i);
				if (data[j] != 0 && sdata[j] != 0)
				{
					dp = sdata[j];
					singleBdata[ord][gct] = sdata[j];
					//cout << "sdt[" << ord << "]" <<"["<<gct<<"]"<< singleBdata[ord][gct] << endl;
			//sum += dp;
					//cout << "dp" << dp << endl;
					//cout<<"gct0"<<gct<<endl;
					gct++;
					
					if (gct > 50)
					{
						
						
							//cout <<"Bdataord=50  " << ord<< "  sumresult" << ushort(sum / gct) << endl;
						

						break;
					}



				}
				/*	else
				{
				cout<<"datai"<< data[j]<<"sdata"<< sdata[j]<<endl;
				}*/

			}
			if (gct >30)
			{
		
			/*	if (Bdata[ord][2] == 0)
					if (gct != 0&&medi!=0)
						Bdata[ord][2] = medi;*/

				break;
			}


		}

		/*if(gct!=0)
			cout << "Bdataord  " << ord << "  sumresult" << ushort(sum / gct) << endl;
*/

		for (i = 0; i<gct; i++) //进行n-1趟
			for (j = 0; j<5 - i; j++)
				if (singleBdata[ord][j]>singleBdata[ord][j + 1])
				{
					temp = singleBdata[ord][j + 1];
					singleBdata[ord][j + 1] = singleBdata[ord][j];
					singleBdata[ord][j] = temp;
				}
		;


		if (gct % 2 == 0)
		{
			medi = (singleBdata[ord][gct / 2] + singleBdata[ord][gct / 2 - 1]) / 2;
		}
		else
		{
			medi = singleBdata[ord][(gct - 1) / 2];
		};
		//cout << "medi" << medi << endl;
		if ((Bdata[ord][2] == 0) && gct >=5 && medi != 0)
			Bdata[ord][2] = medi;
		if (ord <= 2 && Bdata[ord][2]> 2800)
			Bdata[ord][2]=0;
		/*if (ord>1&&Bdata[ord-1][2]> Bdata[ord][2])
			Bdata[ord][2] = 0;*/

		cout << "Bdata[" << ord << "]" << "[" << 0 << "]" << Bdata[ord][0] << endl;
		cout << "Bdata[" << ord << "]" << "[" << 1 << "]" << Bdata[ord][1] << endl;
		cout << "Bdata[" << ord << "]" << "[" << 2 << "]" << Bdata[ord][2] << endl;
		cout << "Bdata[" << ord << "]" << "[" << 3 << "]" << Bdata[ord][3] << endl;
		if (Bdata[ord][2] != 0)
			ord++;

	}



}

void hitdepth(Mat sourceimg, Mat binary, Point center, int difx, int dify, int r)
{
	cout << "in";
	int i = 0, j = 0;
	ushort gct = 0;
	ushort temp = 0;
	ushort medi = 0;
	UINT32 sum = 0;
	UINT16 dp = 0;


		Bdata[ord][0] = center.x;
		Bdata[ord][1] = center.y;
		Bdata[ord][3] = 2 * r;
		for (i = center.y - dify; i <= center.y - dify + r; i++)
		{
			for (j = center.x - difx; j <= center.x - difx + r; j++)

			{
				UINT8* data = binary.ptr<UINT8>(i);
				UINT16* sdata = sourceimg.ptr<UINT16>(i);
				if (data[j] != 0 && sdata[j] != 0)
				{
					dp = sdata[j];
					singleBdata[ord][gct] = sdata[j];
					gct++;

					if (gct > 50)
					{					break;				}
				}
	
			}
				if (gct >30)
				{	break;
		     	}


		}

		for (i = 0; i<gct; i++) //进行n-1趟
			for (j = 0; j<5 - i; j++)
				if (singleBdata[ord][j]>singleBdata[ord][j + 1])
				{
					temp = singleBdata[ord][j + 1];
					singleBdata[ord][j + 1] = singleBdata[ord][j];
					singleBdata[ord][j] = temp;
				}
		;

		if (gct % 2 == 0)
		{
			medi = (singleBdata[ord][gct / 2] + singleBdata[ord][gct / 2 - 1]) / 2;
		}
		else
		{
			medi = singleBdata[ord][(gct - 1) / 2];
		};
		if ((Bdata[ord][2] == 0) && gct >= 5 && medi != 0)
			Bdata[ord][2] = medi;
		if (ord <= 2 && Bdata[ord][2]> 2800)
			Bdata[ord][2] = 0;

		cout << "Bdata[" << ord << "]" << "[" << 0 << "]" << Bdata[ord][0] << endl;
		cout << "Bdata[" << ord << "]" << "[" << 1 << "]" << Bdata[ord][1] << endl;
		cout << "Bdata[" << ord << "]" << "[" << 2 << "]" << Bdata[ord][2] << endl;
		cout << "Bdata[" << ord << "]" << "[" << 3 << "]" << Bdata[ord][3] << endl;
		if (Bdata[ord][2] != 0)
			ord++;

	}




