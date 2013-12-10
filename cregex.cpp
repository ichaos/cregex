//my regex engine based on ken thompson algorithm
//his paper: regular expression search algorithm
#include <string>
#include <iostream>
#include <vector>
#include <stack>
#include <map>

using namespace std;

class NfaNode {
public:
	char input;
	bool final;
	NfaNode *start, *end;
	map<char, vector<NfaNode *> > next;
	NfaNode(): input('\0'), final(false), start(this), end(this) {}
};

//stage 1: check regex syntax, currently only support + * ? ., no eacape
int syntaxSieve(string s)
{
    return 0;
}

int whichType(char c)
{
    return (c == '*' || c == '|' || c == '(' || c == ')');
}

char TRYNEXT(string s, int i) {
	if (i + 1 < s.size())
		return s[i + 1];
	else return '\0';
}

bool UNARYOPE(char c) {
	return c == '?' || c == '*' || c == '+';
}

string postRegex(string r) {
	stack<string> s;
	stack<int> depth;
	stack<char> ops;

	int dd = 0;
	int n = r.size();
	string tmp;
	s.push("");
	for (int i = 0; i < n; i++) {
		switch (r[i]) {
			case '*':
			case '+':
			case '?':
				s.top().append(1, r[i]);
				break;
			case '|':
				depth.push(dd); //here, dd should be 1
				dd = 0;
				ops.push('|');
				break;
			case '(':
				depth.push(dd + 1);
				dd = 0;
				ops.push('(');
				break;
			case ')':
				while (ops.top() != '(') {
					if (ops.top() == '|') {
						tmp = s.top(); s.pop();
						s.top().append(tmp);
						s.top().append("|");
						dd = depth.top(); depth.pop(); //dd === 1 here
					}
					ops.pop();
				}
				ops.pop();
				dd = depth.top(); depth.pop();
				break;
			default:
				s.push(r.substr(i, 1));
				dd++;
				break;
		}
		if (dd == 2 && !UNARYOPE(TRYNEXT(r, i))) {
			tmp = s.top(); s.pop();
			s.top() += tmp;
			s.top() += ".";
			dd = 1;
		}
	}
	//end, check ops '|'
	if (!ops.empty() && ops.top() == '|') {
		tmp = s.top(); s.pop();
		s.top() += tmp;
		s.top() += "|";
		dd = depth.top(); depth.pop();
	}
	if (dd == 2) {
		tmp = s.top(); s.pop();
		s.top() += tmp;
		s.top() += ".";
		dd = 1;
	}
	return s.top();
}

NfaNode *concatenate(NfaNode *a, NfaNode *b) {
	a->end->next[b->input].push_back(b);
	a->end = b->end;
	return a;
}

// *
NfaNode *klee(NfaNode *s, NfaNode *cur) {
	if (!s) s = new NfaNode();
	NfaNode *e = new NfaNode();
	cur->end->next['\0'].push_back(e);
	cur->end->next['\0'].push_back(cur->start);
	s->end->next['\0'].push_back(e);
	s->end->next[cur->input].push_back(cur);
	s->end = e;
	return s;
}

// + 
NfaNode *plusOp(NfaNode *s, NfaNode *cur) {
	if (!s) s = new NfaNode();
	NfaNode *e = new NfaNode();
	s->end->next[cur->input].push_back(cur);
	e->next['\0'].push_back(s);
	cur->end->next['\0'].push_back(e);
	s->end = e;
	return s;
}

// | 
NfaNode *union2node(NfaNode *a, NfaNode *b) {
	NfaNode *s = new NfaNode();
	s->next[a->input].push_back(a);
	s->next[b->input].push_back(b);
	NfaNode *e = new NfaNode();
	a->end->next['\0'].push_back(e);
	b->end->next['\0'].push_back(e);
	s->end = e;
	return s;
}

NfaNode *regexToNFA(string rres) {
	stack<NfaNode *> s;
	NfaNode *n, *p, *e;
	for (int i = 0; i < rres.size(); i++) {
		switch (rres[i]) {
			case '*': //star, klee
				n = s.top(); s.pop();
				p = NULL;
				//if (!s.empty()) {
				//	p = s.top(); s.pop();
				//}
				p = klee(p, n);
				s.push(p);
				break;
			case '+':
				n = s.top(); s.pop();
				p = NULL;
				//if (!s.empty()) {
				//	p = s.top(); s.pop();
				//}
				p = plusOp(p, n);
				s.push(p);
				break;
			case '?': //not support now
				break;
			case '|':
				n = s.top(); s.pop();
				p = s.top(); s.pop();
				p = union2node(p, n);
				s.push(p);
				break;
			case '.':
				n = s.top(); s.pop();
				p = s.top(); s.pop();
				p = concatenate(p, n);
				s.push(p);
				break;
			default: 
				n = new NfaNode();
				e = new NfaNode();
				e->input = rres[i];
				n->next[e->input].push_back(e);
				n->end = e;
				s.push(n);
				break;
		}
	}
	n = s.top();
	if (n->input != '\0') {
		NfaNode *h = new NfaNode();
		h->next[n->input].push_back(n);
		h->end = n->end;
		h->end->final = true;
		return h;
	}
	n->end->final = true;
	return n;
}

vector<pair<int, int> > match(NfaNode *nfa, string s) {
	vector<pair<int, int> > v;
	vector<NfaNode *> active, next;
	int end = -1;
	for (int i = 0; i < s.size(); i++) {
		end = -1;
		active.push_back(nfa);
		for (int j = i; j < s.size(); j++) {
			for (int k = 0; k < active.size(); k++) {
				NfaNode *tmp = active[k];
				//add its out edges into next candidate sets
				//debug: next node
				for (int dd = 0; dd < tmp->next[s[j]].size(); dd++) {
					cout << "eat " << s[j] << ", next: " << tmp->next[s[j]][dd] << " and next->end: " << tmp->next[s[j]][dd]->end << endl;
				}
				next.insert(next.begin(), tmp->next[s[j]].begin(), tmp->next[s[j]].end());
				//parse epsilon transfer
				vector<NfaNode *> epsilon, ne;
				epsilon.insert(epsilon.begin(), tmp->next['\0'].begin(), tmp->next['\0'].end());
				while(!epsilon.empty()) {
					//cout << "epsilon.size() = " << epsilon.size() << endl;
					for (int t = 0; t < epsilon.size(); t++) {
						next.insert(next.begin(), epsilon[t]->next[s[j]].begin(), epsilon[t]->next[s[j]].end());
						ne.insert(ne.begin(), epsilon[t]->next['\0'].begin(), epsilon[t]->next['\0'].end());
					}
					epsilon = ne;
					ne.clear();
				}
			}
			active = next;
			next.clear();
			for (int k = 0; k < active.size() && end < 0; k++) {
				if (active[k]->final) { //find one!
					end = j;
					cout << "----->find one here: " << end << endl;
					break;
				}
				vector<NfaNode *> epsilon, ne;
				epsilon.insert(epsilon.begin(), active[k]->next['\0'].begin(), active[k]->next['\0'].end());
				while(end < 0 && !epsilon.empty()) {
					for (int t = 0; t < epsilon.size(); t++) {
						if (epsilon[t]->final) {
							end = j;
							break;
						}
						next.insert(next.begin(), epsilon[t]->next['\0'].begin(), epsilon[t]->next['\0'].end());
					}
					epsilon = ne;
					ne.clear();
				}
			}
		}
		if (end >= 0) {
			v.push_back(pair<int, int>(i, end));
		}
		active.clear();
		next.clear();
	}
	return v;
}

#define TESTCASENUM 20
string regexTestCase[TESTCASENUM] = {
	"abcd",
	"ab|cd",
	"a(bc|e)*d",
	"a*b*c*d*",
	"a(b*c)*|ed(af)"
};

void postRegex_Test() {
	//standard test case
	for (int i = 0; i < TESTCASENUM; i++) {
		if (regexTestCase[i].empty()) break;
		cout << regexTestCase[i] << " --> " << postRegex(regexTestCase[i]) << endl;
	}
	string s;
    while (1)
    {
        cout << "Please input an regular expression(q to quit): " << endl;
        cin >> s;
        if (s == "q") return;
        cout << "Its postfix expression is: " << postRegex(s) << endl;
    }
}

int main()
{
    //postRegex_Test();
    //return 0;
    while (1)
    {
        string reg;
        string text;
        cout << "Please input regex: " << endl;
        cin >> reg;
        cout << "Please input search text: " << endl;
        cin >> text;
        string rs = postRegex(reg);
        cout << "reverse regular expression: " << rs << endl;
        NfaNode *nfa = regexToNFA(rs);
        vector<pair<int, int> > v = match(nfa, text);
        for (int i = 0; i < v.size(); i++) {
        	cout << "====match from " << v[i].first << " to " << v[i].second << endl;
        }
    }
    return 0;
}
