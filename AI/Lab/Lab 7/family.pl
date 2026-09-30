% Facts

% Alice is a parent of Bob.
parent(alice, bob).

parent(alice, carol).
parent(bob, david).



% rule

% X is a grandparent of Z if X is a parent of Y AND Y is a parent of Z

grandparent(X, Z) :-
    parent(X, Y),
    parent(Y, Z).