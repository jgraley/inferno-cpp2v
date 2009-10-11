//static struct { int m1; } o1;
//struct s4 { int m4; };
//struct s4 o4; 
//s4 o5;

int ghgh()
{
    {
    // When called twice, this detects an old bug where o2 was being made static
  //  struct { int m2; } o2 = { 0 };
    //o2.m2++; 
   // return o2.m2;
    }
    struct s3 { int m3; };
    struct s3 o3;
    s3 o4;
}
/*
int main()
{
    ghgh();
    return ghgh();
}*/
