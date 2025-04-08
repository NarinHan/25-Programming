#include <iostream>
#include <cstring>
#include <cctype>
#include <set>
#include <map>
#include <memory>
#include <stdexcept>

using namespace std ;

enum class BinOp { And, Or, Imp } ;

string binOpToString(BinOp op) 
{
    switch(op) {
        case BinOp::And: return "&&" ;
        case BinOp::Or: return "||" ;
        case BinOp::Imp: return "=>" ;
        default: return "?" ;
    }
}

class Formula
{
public:
    virtual ~Formula () {} 
    virtual string to_string () const = 0 ; 
} ;

class Atom : public Formula 
{
public :
    string name ;
    
    Atom (const string& name) {
        if (name.empty() || !isalpha(name[0])) {
            throw invalid_argument("Atom name must start with a letter.") ;
        }
        for (char ch : name) {
            if (!isalnum(ch)) {
                throw invalid_argument("Atom name must be alphanumeric.") ;
            }
        }
        this->name = name ;
    }
    
    string to_string () const override {
        return name ;
    }
} ;

class Const : public Formula 
{
public :
    bool value ;

    Const (bool value) : value(value) {}

    string to_string () const override {
        return value ? "true" : "false" ;
    }
} ;

class Neg : public Formula
{
public : 
    shared_ptr<Formula> operand ;
    
    Neg (shared_ptr<Formula> operand) : operand(operand) {}

    string to_string () const override {
        return "!(" + operand->to_string() + ")" ;
    }
} ;

class BinFormula : public Formula
{
public :
    BinOp op ;
    shared_ptr<Formula> left ;
    shared_ptr<Formula> right ;

    BinFormula (BinOp op, 
                shared_ptr<Formula> left,
                shared_ptr<Formula> right) : op(op), left(left), right(right) {}

    string to_string () const override {
        return "(" + left->to_string() + " " + binOpToString(op) + " " + right->to_string() + ")" ;
    }
} ;

vector<string> tokenize (const string& input)
{
    vector<string> tokens ;
    string current ;
    bool in_quotes = false ;

    for (size_t i = 0; i < input.length(); i++) {
        char ch = input[i] ;

        if (in_quotes) {
            current += ch ;
            if (ch == '"') {
                in_quotes = false ;
                tokens.push_back(current) ;
                current.clear() ;
            }
        } else if (ch == '"') {
            if (!current.empty()) {
                tokens.push_back(current) ;
                current.clear() ;
            }
            current += ch ;
            in_quotes = true ;
        } else if (isspace(ch)) {
            continue ;
        } else if (ch == '(' || ch == ')' || ch == ',') {
            if (!current.empty()) {
                tokens.push_back(current) ;
                current.clear() ;
            }
            tokens.push_back(string(1, ch)) ; // creates a string containing just one character
        } else {
            current += ch ;
        }
    }

    if (!current.empty()) {
        tokens.push_back(current) ;
    }

    return tokens ;
}

class FormulaBuilder 
{
public:
    FormulaBuilder (const vector<string>& tokens) : tokens(tokens), pos(0) {}

    shared_ptr<Formula> buildFormula () {
        if (pos >= tokens.size()) {
            throw runtime_error("Unexpected end of input") ;
        }

        string token = tokens[pos] ;

        if (token == "pAtom") {
            return pAtom() ;
        } else if (token == "pConst") {
            return pConst() ;
        } else if (token == "pNeg") {
            return pNeg() ;
        } else if (token == "pAnd" || token == "pOr" || token == "pImp") {
            return pBinFormula(token) ;
        } else {
            throw runtime_error("Unexpected token: " + token) ;
        }
    }

private:
    const vector<string>& tokens ;
    size_t pos ;
     
    string consume () {
        if (pos >= tokens.size()) {
            throw runtime_error("Out of tokens") ;
        }
        return tokens[pos++] ;
    }

    void expect (const string& expected) {
        if (consume() != expected) {
            throw runtime_error("Expected '" + expected + "'") ;
        }
    }

    shared_ptr<Formula> pAtom () {
        expect("pAtom") ;
        expect("(") ;
        string name = consume() ;
        expect(")") ;
        return make_shared<Atom>(name.substr(1, name.size() - 2)) ;
    }

    shared_ptr<Formula> pConst () {
        expect("pConst") ;
        expect("(") ;
        string val = consume() ;
        expect(")") ;
        return make_shared<Const>(val == "\"true\"") ;
    }

    shared_ptr<Formula> pNeg () {
        expect("pNeg") ;
        expect("(") ;
        auto operand = buildFormula() ;
        expect(")") ;
        return make_shared<Neg>(operand) ;
    }

    shared_ptr<Formula> pBinFormula (const string& opToken) {
        BinOp op ;
        if (opToken == "pAnd") op = BinOp::And ;
        if (opToken == "pOr") op = BinOp::Or ;
        if (opToken == "pImp") op = BinOp::Imp ;

        expect(opToken) ;
        expect("(") ;
        auto left = buildFormula() ;
        expect(",") ;
        auto right = buildFormula() ;
        expect(")") ;
        return make_shared<BinFormula>(op, left, right) ;
    }
} ;

shared_ptr<Formula> buildFromTokens (const vector<string>& tokens) 
{
    FormulaBuilder builder(tokens) ;
    return builder.buildFormula() ;
}

void collectAtoms (shared_ptr<Formula> f, set<string>& result) 
{
    if (auto atom = dynamic_pointer_cast<Atom>(f)) {
        result.insert(atom->name) ;
    } else if (auto constant = dynamic_pointer_cast<Const>(f)) {
        // do nothing
    } else if (auto neg = dynamic_pointer_cast<Neg>(f)) {
        collectAtoms(neg->operand, result) ;
    } else if (auto bin = dynamic_pointer_cast<BinFormula>(f)) {
        collectAtoms(bin->left, result) ;
        collectAtoms(bin->right, result) ;
    }
}

set<string> getAllAtomicProps (shared_ptr<Formula> formula) 
{
    set<string> result ;
    collectAtoms(formula, result) ;
    return result ;   
}

class FormulaInterpreter 
{
public :
    FormulaInterpreter(shared_ptr<Formula> formula) : formula(formula) {
        set<string> atomSet = getAllAtomicProps(formula) ;
        atoms.assign(atomSet.begin(), atomSet.end()) ;
    }

    bool isSatisfiable ()
    {
        map<string, bool> assignment ;
        return tryAssignments(0, assignment) ;
    }

    bool isValid ()
    {
        map<string, bool> assignment ;
        return tryAllAssignmentsForValidity(0, assignment) ;
    }

private :
    shared_ptr<Formula> formula ;
    vector<string> atoms ;

    bool evaluate (const shared_ptr<Formula> &formula, const map<string, bool>& assignment) 
    {
        if (auto atom = dynamic_pointer_cast<Atom>(formula)) {
            auto it = assignment.find(atom->name) ;
            if (it == assignment.end()) {
                throw runtime_error("Unassigned variable: " + atom->name) ;
            }
            return it->second ;
        } else if (auto constant = dynamic_pointer_cast<Const>(formula)) {
            return constant->value ;
        } else if (auto neg = dynamic_pointer_cast<Neg>(formula)) {
            return !evaluate(neg->operand, assignment) ;
        } else if (auto bin = dynamic_pointer_cast<BinFormula>(formula)) {
            bool left = evaluate(bin->left, assignment) ;
            bool right = evaluate(bin->right, assignment) ;
            
            switch (bin->op) {
                case BinOp::And: return left && right ;
                case BinOp::Or: return left || right ;
                case BinOp::Imp: return !left || right ;
            }
        }

        throw runtime_error("Unknown formula type.") ;
    } 

    bool tryAssignments (int index, map<string, bool>& assignment)
    {
        if (index == atoms.size()) {
            return evaluate(formula, assignment) ;
        }

        string atom = atoms[index] ;

        assignment[atom] = false ;
        if (tryAssignments(index + 1, assignment)) {
            return true ;
        }

        assignment[atom] = true ;
        if (tryAssignments(index + 1, assignment)) {
            return true ;
        }

        return false ;
    }

    bool tryAllAssignmentsForValidity (int index, map<string, bool>& assignment) 
    {
        if (index == atoms.size()) {
            return evaluate(formula, assignment) ;
        }

        const string& atom = atoms[index] ;

        assignment[atom] = false ;
        if (!tryAllAssignmentsForValidity(index + 1, assignment)) {
            return false ;
        }

        assignment[atom] = true ;
        if (!tryAllAssignmentsForValidity(index + 1, assignment)) {
            return false ;
        } 

        return true ;
    }
} ;

int main ()
{
    // this is the input
    string input = R"(pAnd(pAtom("p"), pOr(pAtom("q"), pNeg(pOr(pNeg(pAtom("r")), pConst("true"))))))" ;
    
    // get formula from the AST object
    vector<string> tokens = tokenize(input) ;
    auto formula = buildFromTokens(tokens) ;
    cout << "Parsed formula: " << formula->to_string() << endl ;

    // get the set of atomic propositions from the formula 
    set<string> atomSet = getAllAtomicProps(formula) ;    
    cout << "Atoms: { " ;
    for (const string& atom : atomSet) {
        cout << atom << " " ;
    }
    cout << "}" << endl ;

    // truth-table
    FormulaInterpreter interpreter(formula) ;

    bool satisfiable = interpreter.isSatisfiable() ;
    cout << "Formula is " << (satisfiable ? "satisfiable" : "unsatisfiable") << endl ;

    bool valid = interpreter.isValid() ;
    cout << "Formula is " << (valid ? "valid" : "not valid") << endl ;

    return 0 ;
}