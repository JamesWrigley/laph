function y(args, expr)
    # eval() works exclusively in the global scope, so we have to elevate our
    # arguments before calling it.
    global x = args
    return eval(parse(expr))
end
