#[allow(dead_code)]
#[allow(non_camel_case_types)]
#[repr(C)]
#[derive(Debug)]
enum NodeKind {
    ND_EQ,        // ==
    ND_NEQ,       // !=
    ND_SMALLER,   // <
    ND_SMALLEREQ, // <=
    ND_ADD,       // +
    ND_SUB,       // -
    ND_MUL,       // *
    ND_DIV,       // /
    ND_ASSIGN,    // =
    ND_IF,        // if
    ND_WHILE,     // while
    ND_FOR,       // for
    ND_LVAR,      // Local Variable
    ND_NUM,       // Integer
    ND_RETURN,    // return
}

#[repr(C)]
pub struct Node<'a> {
    kind: NodeKind,
    lhs: &'a Node<'a>,
    rhs: &'a Node<'a>,

    condition: *const Node<'a>,
    initialization: *const Node<'a>,
    increment: *const Node<'a>,
    els: *const Node<'a>,
    then: &'a Node<'a>,

    value: c_int,
    offset: c_int,
}

impl<'a> Node<'a> {
    fn cond(&self) -> Option<&'a Node<'a>> {
        if !self.condition.is_null() {
            unsafe { self.condition.as_ref() }
        } else {
            None
        }
    }

    fn init(&self) -> Option<&'a Node<'a>> {
        if !self.initialization.is_null() {
            unsafe { self.initialization.as_ref() }
        } else {
            None
        }
    }

    fn els(&self) -> Option<&'a Node<'a>> {
        if !self.els.is_null() {
            unsafe { self.els.as_ref() }
        } else {
            None
        }
    }

    fn inc(&self) -> Option<&'a Node<'a>> {
        if !self.increment.is_null() {
            unsafe { self.increment.as_ref() }
        } else {
            None
        }
    }
}

use std::{
    fmt::Debug,
    os::raw::{c_char, c_int},
};
impl Debug for Node<'_> {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        use NodeKind::*;
        match self.kind {
            ND_NUM => f
                .debug_struct("Node")
                .field("kind", &self.kind)
                .field("value", &self.value)
                .finish(),
            ND_LVAR => f
                .debug_struct("Node")
                .field("kind", &self.kind)
                .field("offset", &self.offset)
                .finish(),
            ND_RETURN => f
                .debug_struct("Node")
                .field("kind", &self.kind)
                .field("lhs", &self.lhs)
                .finish(),
            ND_IF => f
                .debug_struct("Node")
                .field("kind", &self.kind)
                .field("cond", &self.cond())
                .field("then", &self.then)
                .field("else", &self.els())
                .finish(),
            ND_WHILE => f
                .debug_struct("Node")
                .field("kind", &self.kind)
                .field("cond", &self.cond())
                .field("then", &self.then)
                .finish(),
            ND_FOR => f
                .debug_struct("Node")
                .field("kind", &self.kind)
                .field("init", &self.init())
                .field("cond", &self.cond())
                .field("inc", &self.inc())
                .field("then", &self.then)
                .finish(),
            _ => f
                .debug_struct("Node")
                .field("kind", &self.kind)
                .field("lhs", &self.lhs)
                .field("rhs", &self.rhs)
                .finish(),
        }
    }
}

#[no_mangle]
pub extern "C" fn hello() {
    println!("hello!!!!! from Rust");
}

#[no_mangle]
pub extern "C" fn ast_print(node: &Node) {
    println!("{:?}", node);
}

// token

#[allow(dead_code)]
#[allow(non_camel_case_types)]
#[repr(C)]
#[derive(Debug)]
enum TokenKind {
    TK_RESERVED, // operator
    TK_IDENT,    // identifier
    TK_RETURN,   // return
    TK_IF,       // if
    TK_WHILE,    // while
    TK_FOR,      // for
    TK_ELSE,     // else
    TK_NUM,      // number literal
    TK_EOF,      // End of File
}

#[repr(C)]
pub struct Token<'a> {
    kind: TokenKind,
    next: &'a Token<'a>,
    value: c_int,
    str: &'a c_char,
    len: c_int,
}

impl Debug for Token<'_> {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        use TokenKind::*;
        match self.kind {
            TK_EOF => f.debug_struct("EOF").finish(),
            TK_NUM => f
                .debug_struct("Token")
                .field("kind", &self.kind)
                .field("value", &self.value)
                .field(
                    "str",
                    &char::from_u32(self.str.to_string().parse().unwrap()).unwrap(),
                )
                .field("len", &self.len)
                .finish(),
            _ => f
                .debug_struct("Token")
                .field("kind", &self.kind)
                .field(
                    "str",
                    &char::from_u32(self.str.to_string().parse().unwrap()).unwrap(),
                )
                .field("len", &self.len)
                .finish(),
        }
    }
}

#[no_mangle]
pub extern "C" fn token_print(token: &Token) {
    println!("{:?}", token);
}

#[repr(C)]
pub struct LVar<'a> {
    next: *const LVar<'a>,
    name: &'a c_char,
    len: c_int,
    offset: c_int,
}

impl Debug for LVar<'_> {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        if let Some(next) = unsafe { self.next.as_ref() } {
            f.debug_struct("LVar")
                .field("next", next)
                .field("name", &self.name)
                .field("len", &self.len)
                .field("offset", &self.offset)
                .finish()
        } else {
            f.debug_struct("LVar is NULL").finish()
        }
    }
}

#[no_mangle]
pub extern "C" fn lvar_print(lvar: &LVar<'_>) {
    println!("RUSTSTART!!!!!!!!!!!!!!!!!!");
    println!("{:?}", lvar);
}
