#ifndef _HNODE_H_
#define _HNODE_H_


#include "llvm/Support/raw_ostream.h"

#include <stack>
#include <algorithm>
#include <unordered_map>
#include <set>

#include "CallExprUtils.h"

using namespace systemc_clang;

//!
//! The hNode class defines the language-neutral HDL opcodes describing
//! systemc constructs and more generally statements and expression that
//! may occur in an SC_MODULE or SC_METHOD. Accessor and print methods
//! are provided.
//!
namespace hnode {

  class hNode;

  typedef hNode * hNodep;

  class hNode {

#define HNODEen \
  etype(hNoop), \
  etype(hModule), \
  etype(hModinitblock), \
  etype(hPortbindings), \
  etype(hPortbinding), \
  etype(hProcesses), \
  etype(hProcess), \
  etype(hMethod), \
  etype(hThread),		 \
  etype(hCStmt), \
  etype(hPortsigvarlist), \
  etype(hPortin), \
  etype(hPortout), \
  etype(hPortio), \
  etype(hSenslist), \
  etype(hSensvar), \
  etype(hSensedge), \
  etype(hTypeinfo), \
  etype(hType), \
  etype(hTypeField), \
  etype(hTypedef), \
  etype(hTypeTemplateParam), \
  etype(hInt), \
  etype(hSigdecl), \
  etype(hVardecl), \
  etype(hVardeclrn), \
  etype(hModdecl), \
  etype(hVarref), \
  etype(hField), \
  etype(hFieldaccess), \
  etype(hVarInit), \
  etype(hVarInitList), \
  etype(hSigAssignL), \
  etype(hSigAssignR), \
  etype(hVarAssign), \
  etype(hBinop), \
  etype(hUnop), \
  etype(hPostfix), \
  etype(hPrefix), \
  etype(hCondop), \
  etype(hMethodCall), \
  etype(hIfStmt), \
  etype(hForStmt), \
  etype(hSwitchStmt), \
  etype(hSwitchCase), \
  etype(hSwitchDefault), \
  etype(hBreak), \
  etype(hContinue), \
  etype(hWhileStmt),   	\
  etype(hDoStmt),   	\
  etype(hReturnStmt),  	\
  etype(hLiteral), \
  etype(hFunction), \
  etype(hThreadFunction), \
  etype(hBuiltinFunction), \
  etype(hFunctionRetType), \
  etype(hFunctionParams), \
  etype(hFunctionParamI), \
  etype(hFunctionParamIO), \
  etype(hWait), \
  etype(hUnimpl), \
  etype(hLast)


  public:

#define etype(x) x
 
    typedef enum { HNODEen } hdlopsEnum;

    //bool is_leaf;
    
    string h_name;
    hdlopsEnum h_op;
    //list<hNodep> child_list;
    std::vector<hNodep> child_list;
 
#undef etype
#define etype(x) #x

    const string hdlop_pn [hLast+1]  =  { HNODEen };

    //hNode() { is_leaf = true;}
    hNode(bool lf) {
      //is_leaf = lf;
      h_op = hdlopsEnum::hNoop;
      h_name = "";
    }

    hNode(hdlopsEnum h) {
      h_op = h;
      h_name = "";
    }
       
  
    hNode(string s, hdlopsEnum h) {
      //is_leaf = true;
      h_op = h;
      h_name = s;
    }

    ~hNode() {
      //return;
      if (!child_list.empty()) {
	//list<hNodep>::iterator it;
	vector<hNodep>::iterator it;
	for (it = child_list.begin(); it != child_list.end(); it++) {
	  /* if (*it) */
	  /*   cout << "child list element " << *it << "\n"; */
	  if (*it) delete *it;
	}
      }
    }

    void set(hdlopsEnum h, string s = "") {
      h_op = h;
      h_name = s;
    }

    void set(string s = "") {
      h_name = s;
    }

    void append(hNodep hnew) {
      child_list.push_back(hnew);
    }

    int size() {
      return child_list.size();
    }
    
    string printopc(hdlopsEnum opc) {
      return hdlop_pn[static_cast<int>(opc)];
    }

    string getname() {
      return h_name;
    }

    hdlopsEnum getopc() {
      return h_op;
    }

   
    // for completeness
    hdlopsEnum str2hdlopenum(string st) {
      const int n = sizeof (hdlop_pn)/sizeof (hdlop_pn[0]);
      for (int i = 0; i < n; i++) {
	if (hdlop_pn[i] == st)
	  return (hdlopsEnum) i;
      }
      return hLast;
    }
    //void print(llvm::raw_fd_ostream & modelout, unsigned int indnt=2) {
    void print(llvm::raw_ostream & modelout=llvm::outs(), unsigned int indnt=2) {
      modelout.indent(indnt);
      modelout << printopc(h_op) << " ";
      if (h_name == "")
	modelout << " NONAME";
      else modelout << h_name;
      if (child_list.empty())
	modelout << " NOLIST\n";
      else {
	modelout << " [\n";
	for (auto child : child_list)
	  if (child)
	    child->print(modelout, indnt+2);
	  else {
	    modelout.indent(indnt+2);
	    modelout << "<null child>\n";
	  }
	modelout.indent(indnt);
	modelout << "]\n";
      }
    }

      // default arguments don't work in lldb
    void dumphcode() {
      print(llvm::outs(), 2);
      LLVM_DEBUG(print(llvm::dbgs(), 2));
    }
  
  };

  //!
  //! The util class provides small utility functions to generate and
  //! recognize C++ and SystemC conformant identifiers.
  //!
  
  class util { 
  public:
    const static int numstr = 7;
    const string scbuiltintype [numstr] = {
      "sc_uint",
      "sc_int",
      "sc_bigint",
      "sc_biguint",
      "sc_bv",
      "sc_logic",
      "sc_clock"
    };
    int scbtlen [ numstr ];

     const set<std::string> sc_built_in_funcs{
       "concat", "wait", "range", "bit", "or_reduce", "xor_reduce", "nor_reduce","and_reduce", "nand_reduce"};
    
    util() {
      for (int i=0; i < numstr; i++)
	scbtlen[i] = scbuiltintype[i].length();
    }
    ~util() {}
    
    static inline void make_ident(string &nm) {
      // https://stackoverflow.com/questions/14475462/remove-set-of-characters-from-the-string
      //str.erase(
      // std::remove_if(str.begin(), str.end(), [](char chr){ return chr == '&' || chr == ' ';}),
      //str.end());
      std::replace(nm.begin(), nm.end(), ' ', '_');
      std::replace(nm.begin(), nm.end(), ':', '_');
      nm.erase(std::remove_if(nm.begin(), nm.end(),
			      [](char c){ return c!='_' && !isalnum(c) ;}), nm.end());

    }

    enum typetocheck {issctype, isscbuiltintype, bothofthem, isscfunc, isscmacro};

    inline void checktypematch(std::string str, const Type *typ, typetocheck t2c){
      bool dotheymatch = false;
      switch (t2c) {
      case issctype: { dotheymatch = isSCType(typ) == isSCType(str); break;}
      case isscbuiltintype: { dotheymatch = isSCType(typ) == isSCBuiltinType(str); break;}
      case bothofthem:  { dotheymatch = isSCType(typ) == (isSCType(str) || isSCBuiltinType(str)); break; }
      case isscfunc: { dotheymatch = isSCType(typ) == isSCFunc(str); break;}
      case isscmacro: {dotheymatch = isSCType(typ) == isSCMacro(str); break;}
      default: ;
      }
      LLVM_DEBUG(llvm::dbgs() << "checktypematch returns " << dotheymatch << "\n");
    }
    
    inline bool isSCType(const Type *typ) {
      static std::vector<llvm::StringRef> sc_dt_ns{"sc_dt"};
      static std::vector<llvm::StringRef> ports_signals_wait{"sc_port_base",
	  "sc_signal_in_if", "sc_signal_out_if", "sc_signal_inout_if",
	  "sc_prim_channel", "sc_thread_process"};
      static std::vector<llvm::StringRef> rvd{"sc_rvd"};

      if (sc_ast_matchers::utils::isInNamespace(typ, sc_dt_ns)) return true;
      if (sc_ast_matchers::utils::isInNamespace(typ, ports_signals_wait)) return true;
      if (sc_ast_matchers::utils::isInNamespace(typ, rvd)) return true;
      return false;
    }
    
    inline bool isSCBuiltinType(const string &tstring, const Type *typ=NULL){
      // linear search sorry, but at least the length
      // isn't hard coded in ...
      bool ret = false;
      bool tmpisnamespace = sc_ast_matchers::utils::isInNamespace(typ, std::vector<llvm::StringRef> {"sc_dt"});
      int found = tstring.find_last_of(" "); // skip qualifiers if any
      for (int i=0; i < numstr; i++) {
	if (tstring.substr(found>=0 ? found+1:0, scbtlen[i]) == scbuiltintype[i]) {
	  ret = true;
	  break;
	}
      }
      if ((typ != NULL) && (tmpisnamespace != ret)) {
	LLVM_DEBUG(llvm::dbgs() << "isSCBuiltinType: '" << tstring << "' (" << tmpisnamespace <<
		   ", " << ret << ")\n");
      }
      return ret;
    }

    inline bool isSCFunc(const string &tstring) {
      return (sc_built_in_funcs.count(tstring)>0);
      // add more as we get them
    }

    inline bool isTypename(const string &tstring) {
      size_t found = tstring.find("typename");
      if (found == string::npos) return false;
      else if (found == (size_t) 0) return true;
      else return false;
    }

    static inline bool isSCType(const string &tstring, const clang::Type *typ = NULL) {
      // linear search and the length is hard coded in ...
      // used in the method name logic.
      // can't use set as we are searching for a substring of tstring
      
     string strings[] = {"sc_in", "sc_rvd", "sc_out", "sc_inout",
			  "sc_signal", "sc_subref", "sc_dt"};
     bool foundsctype = false;
     
     for (string onestring : strings) {
       if (tstring.find(onestring)!=string::npos) {
	 foundsctype = true;
	 break;
       }
       else foundsctype = false;
     }
     if (typ != NULL) {
       bool tmpsctype = sc_ast_matchers::utils::isInNamespace(typ, "sc_dt");
       if (tmpsctype != foundsctype)
	 LLVM_DEBUG(llvm::dbgs() << "isSCType: '" << tstring << "' (" << tmpsctype <<", " << foundsctype << ")\n");

     }
     return foundsctype;
     
      /* if (tstring.substr(0, 6) == "class ") // this is so stupid */
      /* 	tstring = tstring.substr(6, tstring.length() - 6); */
      /* //return (tstring.find("sc_in")!=string::npos)  */
      /* return ((tstring.substr(0, 12) == "sc_core::sc_") || */
      /* 	      (tstring.substr(0, 5) == "sc_in") || */
      /* 	      (tstring.substr(0, 9) == "sc_rvd_in") || */
      /* 	      (tstring.substr(0, 6) == "sc_out") || */
      /* 	      (tstring.substr(0, 10) == "sc_rvd_out") || */
      /* 	      (tstring.substr(0, 8) == "sc_inout") || */
      /* 	      (tstring.substr(0, 9) == "sc_signal") || */
      /* 	      (tstring.substr(0, 6) == "sc_rvd") || */
      /* 	      (tstring.substr(0, 9) == "sc_subref") || */
      /* 	      (tstring.substr(0,5) == "sc_dt")); */
    }


    
    static inline bool isSCMacro(const std::string &str_in) {
      string sc_macro_strings [] = {"sc_min", "sc_max", "sc_abs"};
      for (string str : sc_macro_strings) {
	if (str_in.find(str) != string::npos) return true;
      }
      return false;
    }
    
    static inline bool isposint(const std::string &str) {
      // https://stackoverflow.com/questions/4654636/how-to-determine-if-a-string-is-a-number-with-c
      // towards the middle
      return !str.empty() && str.find_first_not_of("0123456789") == string::npos;
    }

  };

  typedef struct {
    string oldn;
    string newn;
    hNodep h_vardeclp;
    bool referenced;
  } names_t;


  // map from variable declaration p to new name
  //typedef std::map<Decl *, names_t> hdecl_name_map_t;
 
  // map from module instance declaration p to new name
  // typedef std::map<ModuleInstance *, names_t> hmodinst_name_map_t;

  const static std::string gvar_prefix{"_scclang_global_"};
  const static std::string lvar_prefix{"_local_"};
  const static std::string tvar_prefix{"_thread_"};

  inline bool is_sigvar(hNodep hnp) {
    return (hnp->h_op ==  hNode::hdlopsEnum::hVardecl) ||
      (hnp->h_op ==  hNode::hdlopsEnum::hSigdecl);
  }
  
  class name_serve {
  private:
    int cnt;
    string prefix;
  public:
 
  name_serve(string prefx=lvar_prefix) : prefix(prefx), cnt(0){ }
    string newname() {
      return (prefix+to_string(cnt++));
    }
    void set_prefix(string prfx) { prefix = prfx; }
    string get_prefix() { return prefix; }
  };


  //!
  //! The newname_map_t class manages identifier names. For adding an
  //! entry, the identifier information is stored in a map with the
  //! clang AST node pointer as key. A new name for identifier is  
  //! generated, and {original name, new name, hNode pointer}
  //! are stored as the value. Additionally, the hNode of the 
  //! identifier is updated to the new name.
  //! The find entry method returns the new name or empty string.
  //!

  
  template <class T>
    class newname_map_t {
  private:
    name_serve ns;
    std::map<T, names_t> hdecl_name_map;

  public:
    //std::map<T, names_t> hdecl_name_map;
    newname_map_t(string prefix = lvar_prefix) { ns.set_prefix(prefix); }
    void add_entry(T declp, string old_name, hNodep hnp )
    {
      string newn = find_entry_newn(declp, false);
      hnp->set(newn);
      if ( newn== "") {
	// this is a new declaration
	newn = old_name+ns.newname();
	hnp->set(newn);
	names_t names = {old_name, newn, hnp, false};
	hdecl_name_map[declp] = names;
      }
    }
    
    string find_entry_newn(T declp, bool set_ref = false) {
      if (hdecl_name_map.find(declp) == hdecl_name_map.end())
	return "";
      // only set referenced bit for Signals and Variables
      if (set_ref && is_sigvar(hdecl_name_map[declp].h_vardeclp)) hdecl_name_map[declp].referenced = true;
      return hdecl_name_map[declp].newn;
    }

    bool is_referenced(T declp) {
      string newn = find_entry_newn(declp, false);
      if ( newn== "") // doesn't exist
	return false;
      return hdecl_name_map[declp].referenced;
    }

    void reset_referenced() {
      for (auto &mapentry:hdecl_name_map) {
	mapentry.second.referenced = false;
      }
    }

    void set_prefix(string prefix) { ns.set_prefix(prefix); }

    string get_prefix() { return ns.get_prefix(); }

    bool empty() { return hdecl_name_map.empty(); }
    size_t size() { return hdecl_name_map.size(); }
    void clear() {hdecl_name_map.clear(); ns.set_prefix(lvar_prefix);}

    void print(llvm::raw_ostream & modelout=llvm::outs(), unsigned int indnt=2) {
      for( auto entry:hdecl_name_map) {
	modelout << entry.first << " " <<entry.second.newn << "\n";
      }
    }
    
    typename std::map<T, names_t>::iterator begin() { return hdecl_name_map.begin();}
    typename std::map<T, names_t>::iterator end() { return hdecl_name_map.end();}

    // note the pass by value on newmap:
    // need a copy to preserve the inserted values
    // or else a clear or destructor on calling param newmap
    // releases the entries
    void insertall(newname_map_t<T> newmap) {
      hdecl_name_map.insert(newmap.begin(),
			    newmap.end());
    }
 
  };

  typedef newname_map_t<NamedDecl *> hdecl_name_map_t;
  typedef newname_map_t<ModuleInstance *> hmodinst_name_map_t;
  typedef newname_map_t<FunctionDecl *> hsimplefunc_name_map_t;

  // map to record type of the method's class to be used as first parameter in the method call
  typedef std::unordered_map<const CXXMethodDecl *, const Type *> method_object_map_t;
  
  class hfunc_name_map_t:
    public hsimplefunc_name_map_t
  {
  public:
    method_object_map_t methodobjtypemap;
    void insertall(hfunc_name_map_t newmap) {
      hsimplefunc_name_map_t::insertall(newmap);
      methodobjtypemap.insert(newmap.methodobjtypemap.begin(), newmap.methodobjtypemap.end());
    }
    void print(llvm::raw_ostream & modelout=llvm::outs(), unsigned int indnt=2) {
      hsimplefunc_name_map_t::print(modelout, indnt);
      modelout << "Methodobjtypemap follows\n";
      for( auto entry:methodobjtypemap) {
	modelout << entry.first << " " <<entry.second << "\n";
      }
      modelout << "Methodobjtypemap end\n";
	    
    }
  };
  
  typedef std::unordered_map<const CXXMethodDecl *, const CXXMethodDecl *> overridden_method_map_t;
  
  // thread name, reset var name, false|true, ASYNC|SYNC
  typedef std::unordered_map<string, hNodep> resetvar_map_t; 

  
} // end namespace hnode

#endif
