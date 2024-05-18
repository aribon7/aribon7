#include <iostream>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <stack>
#include <string>

using namespace std;

// 定义文法规则
struct Production {
    char nonTerminal;
    vector<string> production;

    // 添加构造函数
    Production(char nt, const vector<string>& prod) : nonTerminal(nt), production(prod) {}
};

// 定义LL(1)语法分析器类
class LL1Parser {
private:
    vector<Production> grammar; // 存储文法规则
    unordered_map<char, unordered_set<char>> firstSet; // 存储FIRST集
    unordered_map<char, unordered_set<char>> followSet; // 存储FOLLOW集
    unordered_map<char, unordered_map<char, string>> parsingTable; // 存储LL(1)分析表

    // 检测是否为终结符
    bool isTerminal(char symbol) {
        return !(symbol >= 'A' && symbol <= 'Z');
    }

    // 消除左递归
    void eliminateLeftRecursion() {
        for (size_t i = 0; i < grammar.size(); ++i) {
            char Ai = grammar[i].nonTerminal;

            // 处理 A -> Aα | β 形式的左递归
            for (size_t j = 0; j < i; ++j) {
                char Aj = grammar[j].nonTerminal;

                vector<string> newProductions;
                vector<string> oldProductions = grammar[i].production;

                for (const string& production : oldProductions) {
                    if (production[0] == Aj) {
                        // 替换 A -> Aα | β 中的 A
                        for (const string& oldProduction : grammar[j].production) {
                            string newProduction = oldProduction + production.substr(1);
                            newProductions.push_back(newProduction);
                        }
                    }
                    else {
                        // 直接添加 A -> β 形式的产生式
                        newProductions.push_back(production + Aj);
                    }
                }

                grammar[i].production = newProductions;
            }

            // 处理直接左递归 A -> Aα | β 形式
            vector<string> newProductions;
            vector<string> oldProductions = grammar[i].production;

            for (const string& production : oldProductions) {
                if (production[0] == Ai) {
                    // 添加新的非左递归的产生式 A -> βA'
                    string newProduction = production.substr(1) + Ai + "'";
                    newProductions.push_back(newProduction);
                }
                else {
                    // 直接添加 A -> β 形式的产生式
                    newProductions.push_back(production + Ai + "'");
                }
            }

            // 添加 A' -> αA' | ε 形式的产生式
            newProductions.push_back("e");
            grammar[i] = Production(Ai, newProductions);
            grammar.push_back(Production(Ai + '\'', oldProductions));
        }
    }

    // 计算FIRST集
    void calculateFirstSet() {
        for (const Production& production : grammar) {
            char nonTerminal = production.nonTerminal;

            for (const string& rightHandSide : production.production) {
                char firstSymbol = rightHandSide[0];

                if (isTerminal(firstSymbol)) {
                    // 如果是终结符，直接添加到FIRST集
                    firstSet[nonTerminal].insert(firstSymbol);
                }
                else {
                    // 如果是非终结符，递归计算FIRST集
                    for (char symbol : firstSet[firstSymbol]) {
                        firstSet[nonTerminal].insert(symbol);
                    }
                }
            }
        }
    }

    // 计算FOLLOW集
    void calculateFollowSet() {
        // 初始化开始符号的FOLLOW集
        followSet[grammar[0].nonTerminal].insert('$');

        // 迭代计算FOLLOW集
        bool changed = true;
        while (changed) {
            changed = false;

            for (const Production& production : grammar) {
                char nonTerminal = production.nonTerminal;

                for (const string& rightHandSide : production.production) {
                    for (int i = 0; i < rightHandSide.length(); i++) {
                        char symbol = rightHandSide[i];

                        if (!isTerminal(symbol)) {
                            // 如果是非终结符
                            int j = i + 1;

                            while (j < rightHandSide.length()) {
                                char nextSymbol = rightHandSide[j];

                                if (isTerminal(nextSymbol)) {
                                    // 如果是终结符，直接添加到FOLLOW集
                                    followSet[symbol].insert(nextSymbol);
                                    break;
                                }
                                else {
                                    // 如果是非终结符，将其FIRST集加入FOLLOW集
                                    for (char nextFirst : firstSet[nextSymbol]) {
                                        if (nextFirst != 'e') {
                                            followSet[symbol].insert(nextFirst);
                                        }
                                    }

                                    // 如果非终结符的FIRST集包含空串，继续检查下一个符号
                                    if (firstSet[nextSymbol].count('e') == 0) {
                                        break;
                                    }
                                }

                                j++;
                            }

                            // 如果右侧最后一个符号是非终结符，将其FOLLOW集加入FOLLOW集
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

    // 构建LL(1)分析表
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
    // 构造函数，传入文法规则
    LL1Parser(const vector<Production>& grammar) : grammar(grammar) {
        // 消除左递归
        eliminateLeftRecursion();

        // 初始化FIRST集、FOLLOW集和LL(1)分析表
        calculateFirstSet();
        calculateFollowSet();
        buildParsingTable();
    }

    // LL(1)语法分析函数
    void parse(const string& input) {
        stack<char> symbolStack;
        symbolStack.push('$'); // 栈底标记
        symbolStack.push(grammar[0].nonTerminal); // 初态非终结符

        string remainingInput = input + "$";
        size_t inputIndex = 0;

        while (!symbolStack.empty()) {
            char topSymbol = symbolStack.top();
            symbolStack.pop();

            if (isTerminal(topSymbol)) {
                // 终结符，匹配输入
                if (topSymbol == remainingInput[inputIndex]) {
                    inputIndex++;
                }
                else {
                    cout << "Error: Unexpected input.\n";
                    return;
                }
            }
            else {
                // 非终结符，使用LL(1)分析表查找产生式
                char inputSymbol = remainingInput[inputIndex];
                string production = parsingTable[topSymbol][inputSymbol];

                if (production.empty()) {
                    cout << "Error: No production found for " << topSymbol << " and " << inputSymbol << ".\n";
                    return;
                }
                else {
                    // 将产生式逆序入栈
                    for (int i = production.length() - 1; i >= 0; i--) {
                        if (production[i] != 'e') { // 'e'表示空串
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
    // 定义文法规则
    vector<Production> grammar = {
        {'E', {"E+T", "T"}},
        {'T', {"T*F", "F"}},
        {'F', {"(E)", "i"}}
    };

    // 创建LL(1)语法分析器
    LL1Parser parser(grammar);

    // 用户输入句子
    cout << "Enter an arithmetic expression: ";
    string input;
    getline(cin, input);

    // 进行LL(1)语法分析
    parser.parse(input);

    return 0;
}