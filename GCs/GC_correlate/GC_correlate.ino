//Patrick McKeen
//pmckeen@g.hmc.edu

int maxCorr(long* arr);
long* correlationList(long gc1, long gc2);
long shift(long gc, int val);
boolean NXOR(boolean a, boolean b);
long dotproduct(long a, long b);
long powerOf2(int v);
String binaryGC(long gc);
long goldCode(int feedback1[], int feedback2[], int seed2);
long shiftRegister(long a, int feedbackList[]);
long nextStep(long a, int feedbackList[]);
long rightmost(int a);


void setup()
{
  Serial.begin(115200);
  int fb1[]={5,2,3,4,5}; //First characteristic polynomial
  int fb2[]={3,3,5};//second
  int seedList[]={1,2,3,4,5,6,7,8};  //The 8 tests
  for(int i=0;i<8;i++)
  {
    long gc1=goldCode(fb1,fb2,seedList[i]);
    for(int j=i;j<8;j++)
    {
      long gc2=goldCode(fb1,fb2,seedList[j]);
      Serial.println("Correlation "+ String(i+1,DEC)+" : "+String(j+1,DEC));
      long* correlation=correlationList(gc1,gc2);//find results
      for(int k=0; k<31;k++)
      {
        Serial.print(String(k,DEC)+": "+String(correlation[k],DEC)+", ");//print results
      }
      Serial.println();//separate lines
      Serial.println("Max: "+String(maxCorr(correlation),DEC)); //new line
      Serial.println();//separate lines
    }
  }

}

void loop()
{
}

int maxCorr(long* arr)
{
  int maximum=-1000;//set starter "max" lower than any potential value in the list
  for(int i=0; i<31; i++)
  {
    maximum=max(maximum,arr[i]); //compare each value to the "max" and set the max equal to the maximum of the two
  }
  return maximum;
}

long* correlationList(long gc1, long gc2)
{
  long correlation[31];//initialize list
  long shiftList[31];//initialize list
  for(int k=0; k<31;k++)
      {
        long test=shift(gc2,k);
        shiftList[k]=test;
        correlation[k]=dotproduct(gc1, test);//run each correlation
      }
      for(int i=0;i<31;i++)
      {
         String str=String(shiftList[i],DEC);//I DON'T KNOW WHY THIS WORKS, BUT THE PROGRAM DOESN'T WORK WITHOUT IT.
      }
      return correlation;
}

long shift(long gc, int val)
{
  if (val==0)
  {
    return gc;//zero shift
  }
  if (val==1)
  {
    return ((rightmost(gc)*powerOf2(29))+(gc>>1));//single shift
  }
  return shift(((rightmost(gc)*powerOf2(29))+(gc>>1)), val-1);//recurse the shift function
}

boolean NXOR(boolean a, boolean b)
{
return ((a&&b)||((!a)&&(!b))); //(a AND b) OR ((NOT a) AND (NOT b))-->(either both or neither)
}

long dotproduct(long a, long b)
{
  int dp=0;//start at zero
  for(int i=0;i<31;i++)
  {
    long p=powerOf2(i);
    boolean bitA=((p&a)!=0);//find the bit at the desired binary digit for a
    boolean bitB=((p&b)!=0);//find the bit at the desired binary digit for b
    dp-=1; //subtract one, assuming they do not match
    if (NXOR(bitA,bitB))
    {
     dp+=2; //if they do match, add 2, making it +1
    }
  }
  return dp;
}

long powerOf2(int v) //recursively finds the value of 2**x
{
  if(v==0)
  {
    return long(1); 
  }
  return long(2)*powerOf2(v-1); 
}

String binaryGC(long gc)
{
  String str1="";//starts with empty string
  for (int i=0; i<31; i++)
  {
    long andVal=gc&(powerOf2(i)); //see if gc's binary code includes 2**i
    int includesDigit=(andVal>0); //if this is greater than 0, gc includes 2**i
    String add="0";
    if (includesDigit)
    {
      add="1";  //if so, then gc includes 2**i and a 1 should be added to the GC
    }
    str1=add+str1; //pre-appended to string
  }
  return str1;
}



long goldCode(int feedback1[], int feedback2[], int seed2) //returns the numerical value of the gold code
{
  long reg1=shiftRegister(1, feedback1);
  long reg2=shiftRegister(seed2, feedback2);
  return reg1^reg2; //ZORs the 2 outputs of the LFSRs together
}



long shiftRegister(long a, int feedbackList[]) //finds the shift register for the LFSR. a is the seed.
{
  long reg=0;
  reg+=(rightmost(a)*powerOf2(30));
  long b=nextStep(a, feedbackList);
  int i=1;
  while((b-a)!=0)
  {
    reg+=(rightmost(b)*powerOf2(30-i));//takes the rightmost digit of each iteration and adds it to the binary string by multiplying by 2**i
    b=nextStep(b, feedbackList); //iterates to the next step
    i++; //iterates
  }
  return reg;
}

long nextStep(long a, int feedbackList[])//finds the next value outpu of the LFSR
{
  long r=a>>1;//shifts one place to the right
  boolean addOne=0;
  for (int i=1; i<feedbackList[0];i++)
  {
   int n=(((powerOf2(5-feedbackList[i]))&a)>0); //tests if the feedback terminals are outputting 1s
   addOne^=n; //XORs the outputs of the feedback terminals together
  }
  return (addOne*16)+r; //generates next iteration
}

long rightmost(int a) //returns whether the rightmost bit is 0 or 1
{
  return (a&1); 
}


