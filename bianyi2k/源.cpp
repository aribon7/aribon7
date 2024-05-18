#include <iostream>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <stack>
#include <string>

using namespace std;

// �����ķ�����
struct Production {
    char nonTerminal;
    vector<string> production;

    // ��ӹ��캯��
    Production(char nt, const vector<string>& prod) : nonTerminal(nt), production(prod) {}
};

// ����LL(1)�﷨��������
class LL1Parser {
private:
    vector<Production> grammar; // �洢�ķ�����
    unordered_map<char, unordered_set<char>> firstSet; // �洢FIRST��
    unordered_map<char, unordered_set<char>> followSet; // �洢FOLLOW��
    unordered_map<char, unordered_map<char, string>> parsingTable; // �洢LL(1)������

    // ����Ƿ�Ϊ�ս��
    bool isTerminal(char symbol) {
        return !(symbol >= 'A' && symbol <= 'Z');
    }

    // ������ݹ�
    void eliminateLeftRecursion() {
        for (size_t i = 0; i < grammar.size(); ++i) {
            char Ai = grammar[i].nonTerminal;

            // ���� A -> A�� | �� ��ʽ����ݹ�
            for (size_t j = 0; j < i; ++j) {
                char Aj = grammar[j].nonTerminal;

                vector<string> newProductions;
                vector<string> oldProductions = grammar[i].production;

                for (const string& production : oldProductions) {
                    if (production[0] == Aj) {
                        // �滻 A -> A�� | �� �е� A
                        for (const string& oldProduction : grammar[j].production) {
                            string newProduction = oldProduction + production.substr(1);
                            newProductions.push_back(newProduction);
                        }
                    }
                    else {
                        // ֱ����� A -> �� ��ʽ�Ĳ���ʽ
                        newProductions.push_back(production + Aj);
                    }
                }

                grammar[i].production = newProductions;
            }

            // ����ֱ����ݹ� A -> A�� | �� ��ʽ
            vector<string> newProductions;
            vector<string> oldProductions = grammar[i].production;

            for (const string& production : oldProductions) {
                if (production[0] == Ai) {
                    // ����µķ���ݹ�Ĳ���ʽ A -> ��A'
                    string newProduction = production.substr(1) + Ai + "'";
                    newProductions.push_back(newProduction);
                }
                else {
                    // ֱ����� A -> �� ��ʽ�Ĳ���ʽ
                    newProductions.push_back(production + Ai + "'");
                }
            }

            // ��� A' -> ��A' | �� ��ʽ�Ĳ���ʽ
            newProductions.push_back("e");
            grammar[i] = Production(Ai, newProductions);
            grammar.push_back(Production(Ai + '\'', oldProductions));
        }
    }

    // ����FIRST��
    void calculateFirstSet() {
        for (const Production& production : grammar) {
            char nonTerminal = production.nonTerminal;

            for (const string& rightHandSide : production.production) {
                char firstSymbol = rightHandSide[0];

                if (isTerminal(firstSymbol)) {
                    // ������ս����ֱ����ӵ�FIRST��
                    firstSet[nonTerminal].insert(firstSymbol);
                }
                else {
                    // ����Ƿ��ս�����ݹ����FIRST��
                    for (char symbol : firstSet[firstSymbol]) {
                        firstSet[nonTerminal].insert(symbol);
                    }
                }
            }
        }
    }

    // ����FOLLOW��
    void calculateFollowSet() {
        // ��ʼ����ʼ���ŵ�FOLLOW��
        followSet[grammar[0].nonTerminal].insert('$');

        // ��������FOLLOW��
        bool changed = true;
        while (changed) {
            changed = false;

            for (const Production& production : grammar) {
                char nonTerminal = production.nonTerminal;

                for (const string& rightHandSide : production.production) {
                    for (int i = 0; i < rightHandSide.length(); i++) {
                        char symbol = rightHandSide[i];

                        if (!isTerminal(symbol)) {
                            // ����Ƿ��ս��
                            int j = i + 1;

                            while (j < rightHandSide.length()) {
                                char nextSymbol = rightHandSide[j];

                                if (isTerminal(nextSymbol)) {
                                    // ������ս����ֱ����ӵ�FOLLOW��
                                    followSet[symbol].insert(nextSymbol);
                                    break;
                                }
                                else {
                                    // ����Ƿ��ս��������FIRST������FOLLOW��
                                    for (char nextFirst : firstSet[nextSymbol]) {
                                        if (nextFirst != 'e') {
                                            followSet[symbol].insert(nextFirst);
                                        }
                                    }

                                    // ������ս����FIRST�������մ������������һ������
                                    if (firstSet[nextSymbol].count('e') == 0) {
                                        break;
                                    }
                                }

                                j++;
                            }

                            // ����Ҳ����һ�������Ƿ��ս��������FOLLOW������FOLLOW��
                            if (j == rightHandSide.length()) {
                                for (char follow : followSet[nonTerminal]) {
                                    followSet[symbol].insert(follow);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // ����LL(1)������
    void buildParsingTable() {
        for (const Production& production : grammar) {
            char nonTerminal = production.nonTerminal;

            for (const string& rightHandSide : production.production) {
                unordered_set<char> firstOfRHS;

                for (char symbol : rightHandSide) {
                    if (isTerminal(symbol)) {
                        firstOfRHS.insert(symbol);
                        break;
                    }
                    else {
                        for (char first : firstSet[symbol]) {
                            if (first != 'e') {
                                firstOfRHS.insert(first);
                            }
                        }

                        if (firstSet[symbol].count('e') == 0) {
                            break;
                        }
                    }
                }

                for (char symbol : firstOfRHS) {
                    if (symbol != 'e') {
                        parsingTable[nonTerminal][symbol] = rightHandSide;
                    }
                    else {
                        for (char follow : followSet[nonTerminal]) {
                            parsingTable[nonTerminal][follow] = rightHandSide;
                        }
                    }
                }
            }
        }
    }

public:
    // ���캯���������ķ�����
    LL1Parser(const vector<Production>& grammar) : grammar(grammar) {
        // ������ݹ�
        eliminateLeftRecursion();

        // ��ʼ��FIRST����FOLLOW����LL(1)������
        calculateFirstSet();
        calculateFollowSet();
        buildParsingTable();
    }

    // LL(1)�﷨��������
    void parse(const string& input) {
        stack<char> symbolStack;
        symbolStack.push('$'); // ջ�ױ��
        symbolStack.push(grammar[0].nonTerminal); // ��̬���ս��

        string remainingInput = input + "$";
        size_t inputIndex = 0;

        while (!symbolStack.empty()) {
            char topSymbol = symbolStack.top();
            symbolStack.pop();

            if (isTerminal(topSymbol)) {
                // �ս����ƥ������
                if (topSymbol == remainingInput[inputIndex]) {
                    inputIndex++;
                }
                else {
                    cout << "Error: Unexpected input.\n";
                    return;
                }
            }
            else {
                // ���ս����ʹ��LL(1)��������Ҳ���ʽ
                char inputSymbol = remainingInput[inputIndex];
                string production = parsingTable[topSymbol][inputSymbol];

                if (production.empty()) {
                    cout << "Error: No production found for " << topSymbol << " and " << inputSymbol << ".\n";
                    return;
                }
                else {
                    // ������ʽ������ջ
                    for (int i = production.length() - 1; i >= 0; i--) {
                        if (production[i] != 'e') { // 'e'��ʾ�մ�
                            symbolStack.push(production[i]);
                        }
                    }
                }
            }
        }

        cout << "Input accepted.\n";
    }
};

int main() {
    // �����ķ�����
    vector<Production> grammar = {
        {'E', {"E+T", "T"}},
        {'T', {"T*F", "F"}},
        {'F', {"(E)", "i"}}
    };

    // ����LL(1)�﷨������
    LL1Parser parser(grammar);

    // �û��������
    cout << "Enter an arithmetic expression: ";
    string input;
    getline(cin, input);

    // ����LL(1)�﷨����
    parser.parse(input);

    return 0;
}