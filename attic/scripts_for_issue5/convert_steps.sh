#! /bin/bash -ex

# s3 deals with MakePatternPtr<X> a, ...;
# Only pulls out a, so may need to apply multiple times
sed -E -i $1 -f s3.sed
sed -E -i $1 -f s3.sed
sed -E -i $1 -f s3.sed
sed -E -i $1 -f s3.sed
sed -E -i $1 -f s3.sed
sed -E -i $1 -f s3.sed
sed -E -i $1 -f s3.sed
sed -E -i $1 -f s3.sed
sed -E -i $1 -f s3.sed
sed -E -i $1 -f s3.sed

# s2 deals with MakePatternPtr<X> a;
# either because it was like that originally or due s3
sed -E -i $1 -f s2.sed

# s1 deals with MakePatternPtr<X> a(args...);
sed -E -i $1 -f s1.sed
