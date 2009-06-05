struct { int m1; } o1;
struct s4 { int m4; };
//struct s4 o4; TODO inferno generates a seperate "s4" identifier node for this, which is totally wrong, it should find the existing one! 
s4 o5;

void ghgh()
{
    {
    struct { int m2; } o2;
    }
    struct s3 { int m3; };
    struct s3 o3;
    s3 o4;
}
