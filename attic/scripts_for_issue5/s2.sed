s/^(\s*)(MakePatternPtr<.*>)\s([a-zA-Z0-9_]+);+((\s*\/\/.*)|\s*)$/\1auto \3 = \2();\5/
