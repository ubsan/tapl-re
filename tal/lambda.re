type term =
| Term_marker
| Term_unit
| Term_var(string)
| Term_abs(string, term)
| Term_app(term, term);

/*
type ty =
| Ty_unit
| Ty_lam(ty, ty);
*/

type ast =
| Ast_marker
| Ast_unit
| Ast_var(int)
| Ast_abs(string, ast)
| Ast_app(ast, ast);

let unit = () => Term_unit;
let marker = () => Term_marker;
let var = (name) => Term_var(name);
let abs = (name, body) => Term_abs(name, body);
let app = (callee, parm) => Term_app(callee, parm);


let rec print_term = (term) => {
  let is_cmplx = fun
  | Term_marker => false
  | Term_unit => false
  | Term_var(_) => false
  | _ => true;

  switch(term) {
  | Term_marker => print_char('@')
  | Term_unit => print_string("()")
  | Term_var(name) => print_string(name)
  | Term_abs(var, body) =>
    print_string("/" ++ var ++ ".");
    print_term(body);
  | Term_app(callee, parm) =>
    if (is_cmplx(callee)) { print_char('(') };
    print_term(callee);
    if (is_cmplx(callee)) { print_char(')') };
    print_char(' ');
    if (is_cmplx(parm)) { print_char('(') };
    print_term(parm);
    if (is_cmplx(parm)) { print_char(')') };
  }
};

let print_ast = (ast) => {
  let rec bound_var_name = (idx, names) => switch (names) {
  | [name, ...names] =>
    if (idx == 0) { name } else { bound_var_name(idx - 1, names) }
  | [] => failwith("malformed lambda calculus ast")
  };

  let is_cmplx = (ast) => switch (ast) {
  | Ast_marker => false
  | Ast_unit => false
  | Ast_var(_) => false
  | _ => true
  };

  let rec print_ast_rec = (ast, names) => {
    let print_cmplx = (ast) => {
      if (is_cmplx(ast)) {
        print_char('('); print_ast_rec(ast, names); print_char(')')
      } else {
        print_ast_rec(ast, names)
      }
    };
    switch (ast) {
    | Ast_marker => print_char('@')
    | Ast_unit => print_string("()")
    | Ast_var(idx) => print_string(bound_var_name(idx, names))
    | Ast_app(callee, parm) =>
      print_cmplx(callee);
      print_char(' ');
      print_cmplx(parm)
    | Ast_abs(name, body) =>
      print_char('/');
      print_string(name);
      print_char('.');
      print_ast_rec(body, [name, ...names])
    }
  };

  print_ast_rec(ast, [])
};

type type_error =
| Te_variable_not_found(string);
exception Type_error(type_error);

let finish = (tm) => {
  let rec get_var = (name, names, idx) => switch (names) {
  | [x, ..._] when x == name => Ast_var(idx)
  | [_, ...xs] => get_var(name, xs, idx + 1)
  | [] => raise(Type_error(Te_variable_not_found(name)))
  };
  let rec finish_rec = (tm, names) => {
    switch (tm) {
    | Term_marker => Ast_marker
    | Term_unit => Ast_unit
    | Term_var(name) => get_var(name, names, 0)
    | Term_app(callee, parm) =>
      Ast_app(finish_rec(callee, names), finish_rec(parm, names))
    | Term_abs(name, body) =>
      Ast_abs(name, finish_rec(body, [name, ...names]))
    }
  };
  finish_rec(tm, [])
};

let substitute = (body, parm) => {
  let rec sub_rec = (body, parm, idx) => {
    switch (body) {
    | Ast_var(idx') when idx == idx' => parm
    | Ast_app(callee', parm') =>
      Ast_app(sub_rec(callee', parm, idx), sub_rec(parm', parm, idx))
    | Ast_abs(name, body') =>
      Ast_abs(name, sub_rec(body', parm, idx + 1))
    | unchanged => unchanged
    }
  };
  sub_rec(body, parm, 0)
};

let rec eval1 = (ast) => {
  let rec eval_app = (callee, parm) => {
    switch (callee) {
    | Ast_marker => None
    | Ast_unit => None
    | Ast_var(_) => failwith("malformed lambda ast")
    | Ast_abs(_, body) => Some(substitute(body, parm))
    | Ast_app(callee', parm') =>
      switch (eval_app(callee', parm')) {
      | Some(callee) => Some(Ast_app(callee, parm))
      | None =>
        switch (eval1(parm)) {
        | Some(parm) => Some(Ast_app(callee, parm))
        | None => None
        }
      }
    }
  };

  switch (ast) {
  | Ast_marker => None
  | Ast_unit => None
  | Ast_var(_) => failwith("malformed lambda ast")
  | Ast_abs(_, _) => None
  | Ast_app(callee, parm) => eval_app(callee, parm)
  }
};

module Type = {};
