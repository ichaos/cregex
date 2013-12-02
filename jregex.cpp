//my regex engine based on ken thompson algorithm
//his paper: regular expression search algorithm
#include <string>
#include <iostream>
#include <vector>
#include <stack>
#include <map>

using namespace std;

class NFANode
{
public:
    char input;
    bool isEnd;
    map<char, NFANode *> jumpTable;
    NFANode()
    {
        isEnd = false;
    }
};

struct basicUnit
{
    vector<char> chars;
};

using namespace std;

//stage 1: check regex syntax, currently only support + * ? ., no eacape
int syntaxSieve(string s)
{
    return 0;
}

int whichType(char c)
{
    return (c == '*' || c == '|' || c == '(' || c == ')');
}

//stage 2: transform regex to reverse polish notation
// a(bc|e)*d --> abc.e|*.d.
vector<basicUnit> rpn(string s)
{
    stack<char> ops;
    vector<basicUnit> rpnexp;
    for (int i = 0; i < s.size(); i++)
    {
        int type = whichType(s[i]);
        switch (type)
        {
        //normal character
        case 0:
        {
            basicUnit b;
            b.chars.push_back(s[i]);
            while (i + 1 < s.size() && whichType(s[i + 1]) == 0)
            {
                i++;
                if (i == s.size() - 1 || s[i + 1] != '*')
                    b.chars.push_back(s[i]);
                else
                {
                    i--;
                    break;
                }
            }
            rpnexp.push_back(b);
            break;
        }
        case 1:
        {
            if (s[i] == '(')
            {
                ops.push(s[i]);
            }
            else if (s[i] == ')')
            {
                while (!ops.empty() && ops.top() != '(')
                {
                    basicUnit b;
                    b.chars.push_back(ops.top());
                    ops.pop();
                    rpnexp.push_back(b);
                }
                ops.pop();
            }
            else if (s[i] == '*')
            {
                basicUnit b;
                b.chars.push_back('*');
                rpnexp.push_back(b);
            }
            else
            {
                while (!ops.empty() && ops.top() != '(')
                {
                    basicUnit b;
                    b.chars.push_back(ops.top());
                    ops.pop();
                    rpnexp.push_back(b);
                }
                ops.push(s[i]);
            }
        } break;
        default: break;
        }
    }
    while (!ops.empty())
    {
        if (ops.top() != '(')
        {
            basicUnit b;
            b.chars.push_back(ops.top());
            rpnexp.push_back(b);
        }
        ops.pop();
    }
    return rpnexp;
}

string re2rpn(string rs)
{
    string rpn = "";
    stack<int> nnode;
    stack<char> ops;
    int nn = 0;
    int n = rs.size();
    for (int i = 0; i < n; i++)
    {
        switch (rs[i])
        {
        case '*':
        case '+':
        case '?':
            if (rpn[rpn.size() - 1] == '.')
            {
                rpn[rpn.size() - 1] = rs[i];
                rpn += '.';
            }
            else
                rpn += rs[i];
            break;
        case '|':
            ops.push('|'); break;
        case '(':
            nnode.push(nn + 1);
            nn = 0;
            break;
        case ')':
            nn = nnode.top();
            nnode.pop();
            if (nn == 2)
            {
                rpn += '.';
                nn = 1;
            }
            break;
        default:
            nn++;
            rpn += rs[i];
            break;
        }
        if (nn == 2)
        {
            if (ops.empty())
                rpn += '.';
            else
            {
                rpn += ops.top();
                ops.pop();
            }
            nn = 1;
        }
    }
    return rpn;
}

struct endNFANode
{
    vector<NFANode *> tails;
};

//stage 3: build nfa based on rpn of regex
NFANode *regexToNFA(vector<basicUnit> &regex)
{
    stack<NFANode *> s;
    stack<endNFANode> se;
    for (int i = 0; i < regex.size(); i++)
    {
        int bus = regex[i].chars.size();
        int j = 0;
        NFANode *last = NULL, *head = NULL;
        if (bus > 1 || whichType(regex[i].chars[0]) == 0)
        {
            for (; j < regex[i].chars.size(); j++)
            {
                NFANode *node = new NFANode();
                node->input = regex[i].chars[j];
                if (last)
                {
                    last->jumpTable[regex[i].chars[j]] = node;
                    last = node;
                }
                else
                {
                    last = head = node;
                }
            }
            endNFANode enfa;
            enfa.tails.push_back(last);
            se.push(enfa);
            s.push(head);
        }
        else
        {
            //FIXME: find a way to connect NFANode effectively
            if (regex[i].chars[j] == '*')
            {
                NFANode *next = s.top(); s.pop();
                NFANode *cnode = new NFANode();
                cnode->input = '\0';
                cnode->jumpTable[next->input] = next;
                endNFANode ne = se.top(); se.pop();
                for (int k = 0; k < ne.tails.size(); k++)
                {
                    ne.tails[k]->jumpTable['\0'] = cnode;
                }
                ne.tails.clear();
                ne.tails.push_back(cnode);
                se.push(ne);
                s.push(cnode);
            }
            else if (regex[i].chars[j] == '|')
            {
                NFANode *right = s.top(); s.pop();
                NFANode *left = s.top(); s.pop();
                NFANode *cnode = new NFANode();
                cnode->input = '\0';
                cnode->jumpTable[left->input] = left;
                cnode->jumpTable[right->input] = right;
                endNFANode righte = se.top(); se.pop();
                endNFANode lefte = se.top(); se.pop();
                endNFANode ne;
                ne.tails.insert(ne.tails.end(), righte.tails.begin(), righte.tails.end());
                ne.tails.insert(ne.tails.end(), lefte.tails.begin(), lefte.tails.end());
                se.push(ne);
                s.push(cnode);
            }
        }
    }
    endNFANode ne = se.top(); se.pop();
    for (int k = 0; k < ne.tails.size(); k++)
    {
        ne.tails[k]->isEnd = true;
        cout << "input char of end: " << ne.tails[k]->input << endl;
    }
    NFANode *last = s.top(); s.pop();
    while (!s.empty())
    {
        NFANode *cur = s.top(); s.pop();
        endNFANode cure = se.top(); se.pop();
        for (int k = 0; k < cure.tails.size(); k++)
        {
            cure.tails[k]->jumpTable[last->input] = last;
        }
        last = cur;
    }
    NFANode *nfa = new NFANode();
    nfa->input = '\0';
    nfa->jumpTable[last->input] = last;
    return nfa;
}

void rpn_test()
{
    //test rpn
    string s;
    while (1)
    {
        cout << "Please input an regular expression(q to quit): " << endl;
        cin >> s;
        if (s == "q") return;
        string rpnexp = "";
        vector<basicUnit> bu = rpn(s);
        for (int i = 0; i < bu.size(); i++)
        {
            string temp = "";
            for (int j = 0; j < bu[i].chars.size(); j++)
            {
                temp = temp + bu[i].chars[j];
            }
            if (bu[i].chars.size() > 1)
                temp = "(" + temp + ")";
            rpnexp += temp;
        }
        cout << "Its rpn expression is: " << rpnexp << endl;
    }
}

void rpn_test(string r)
{
    string rpnexp = "";
    vector<basicUnit> bu = rpn(r);
    for (int i = 0; i < bu.size(); i++)
    {
        string temp = "";
        for (int j = 0; j < bu[i].chars.size(); j++)
        {
            temp = temp + bu[i].chars[j];
        }
        if (bu[i].chars.size() > 1)
            temp = "(" + temp + ")";
        rpnexp += temp;
    }
    cout << "Its rpn expression is: " << rpnexp << endl;
}

void nfsTest(string text, NFANode *p)
{
    //current legal state list
    vector<NFANode *> next, cur;
    vector<NFANode *> *np = &next, *cp = &cur;
    for (int k = 0; k < text.size(); k++)
    {
        cp->clear();
        cp->push_back(p);
        int end = -1;
        for (int i = k; i < text.size(); i++)
        {
            np->clear();
            for (int j = 0; j < cp->size(); j++)
            {
                if ((*cp)[j]->isEnd)
                {
                    end = i;
                    cout << "====find match from " << k + 1 << " to " << end << endl;
                }
                if ((*cp)[j]->jumpTable.find(text[i]) != (*cp)[j]->jumpTable.end())
                {
                    cout << "match " << text[i] << " at index " << i << endl;
                    cout << "Its output size: " << (*cp)[j]->jumpTable[text[i]]->jumpTable.size() << endl;
                    np->push_back((*cp)[j]->jumpTable[text[i]]);
                }
                else
                {
                    NFANode *cnode = (*cp)[j];
                    while (cnode->jumpTable.find('\0') != cnode->jumpTable.end())
                    {
                        cnode = cnode->jumpTable['\0'];
                        cout << "control node @ " << cnode << ", size of output: " << cnode->jumpTable.size() << endl;
                        if (cnode->jumpTable.find(text[i]) != cnode->jumpTable.end())
                        {
                            np->push_back(cnode->jumpTable[text[i]]);
                        }
                    }
                }
            }
            if (np->size() == 0) break;
            vector<NFANode *> *tmp = cp;
            cp = np;
            np = tmp;
        }
        //find a match
        for (int j = 0; j < cp->size(); j++)
        {
            NFANode *cnode = (*cp)[j];
            while (cnode)
            {
                if (cnode->isEnd)
                {
                    cout << "====find match from " << k + 1 << " to " << text.size() << endl;
                    break;
                }
                if (cnode->jumpTable.find('\0') != cnode->jumpTable.end())
                    cnode = cnode->jumpTable['\0'];
                else
                    cnode = NULL;
            }
        }
    }
}

void re2rpn_Test()
{
    string s;
    while (1)
    {
        cout << "Please input an regular expression(q to quit): " << endl;
        cin >> s;
        if (s == "q") return;
        cout << "Its rpn expression is: " << re2rpn(s) << endl;
    }
}

int main()
{
    //re2rpn_Test();
    //return 0;
    while (1)
    {
        string reg;
        string text;
        cout << "Please input regex: " << endl;
        cin >> reg;
        cout << "Please input search text: " << endl;
        cin >> text;
        vector<basicUnit> bu = rpn(reg);
        rpn_test(reg);
        NFANode *nfa = regexToNFA(bu);
        nfsTest(text, nfa);
    }
    return 0;
}
