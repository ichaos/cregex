//reverse polish notation
#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <stack>

using namespace std;


int getType(char c) {
	return (c == '(' || c == ')' || c == '+' || c == '-' || c == '*' || c == '/');
}

string rpn(string exp) {
	stack<char> s1, s2;
	map<char, int> opPriority;

	opPriority['#'] = 0;
	opPriority['+'] = 2;
	opPriority['-'] = 2;
	opPriority['*'] = 3;
	opPriority['/'] = 3;
	opPriority['('] = opPriority[')'] = 1;

	s1.push('#');

	for (int i = 0; i < exp.size(); i++) {
		int type = getType(exp[i]);
		switch (type) {
			//operand
			case 0: s2.push(exp[i]); break;
			//operator
			case 1: 
			if (exp[i] == ')') {
				while (s1.top() != '(') {
					s2.push(s1.top());
					s1.pop();
				}
				s1.pop();
			} else if (exp[i] == '(') {
				s1.push(exp[i]);
			} else {
				//compare priority 
				while (opPriority[s1.top()] >= opPriority[exp[i]]) {
					s2.push(s1.top());
					s1.pop();
				}
				s1.push(exp[i]);
			}
			break;
			default: break;
		}
	}

	while (s1.top() != '#') {
		s2.push(s1.top());
		s1.pop();
	}

	string ret = "";
	while (!s2.empty()) {
		ret = s2.top() + ret;
		s2.pop();
	}
	return ret;
}

int main() {
	string s;
	while (1) {
		cout << "Please input infix expression: " << endl;
		cin >> s;
		if (s == "q") break;
		cout << "RPN of " << s << " is " << rpn(s) << endl;
	}
	return 0;
}