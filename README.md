# complex-recursive-descent-parser-cpp

- Developed it in C++ for HTML language. This program has two parts: lexical analyzer and syntactic analyzer. It uses concept of binary parse tree.
- It is able to catch all syntax errors in imputed HTML language files and gives suggestion of what is missing and causing the error.

If no errors exists then it prints out:
- Differnt tages and total numbers occurence (h1,h2, p, title, i etc)
- extracts text strings between tags
- Maximum nesting depth of those text string.
- 

Program is able to skip html comments and white spaces (not inside html text string).


examples

input 1
<title>this should work</author>
it is syntactically correct
<br/>
<p>this too</p>
and the rest of it


output 1
start tag title does not match end tag author


input 2
<h1>Hello</h1>
<p>yeah this is text</p>
<h2>there</h2>
<h4>this is a jump</h4>
<h1>How</h1>
<h2>are</h2>
<h3>you</h3>
<h2>doing</h2>
<h2>along with me <b>this</b></h2>
<h3>anyway</h3>
<h1>???</h1>

output 2

Maximum nesting depth is 2
b: 1
h1: 3
h2: 4
h3: 2
h4: 1
p: 1
=====================
1: Hello 
1.1: there 
1.1.0.1: this is a jump 
2: How 
2.1: are 
2.1.1: you 
2.2: doing 
2.3: along with me   
2.3.1: anyway 
3: ??? 


