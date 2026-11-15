%% ======================================================================
%% royal_archive_access.pl
%% Problem 3: Royal Archive - Access Verification using Resolution Refutation
%%
%% Run:  swipl -q -s royal_archive_access.pl -g main -t halt
%% ======================================================================
%%
%% VOCABULARY
%%   Constants : mira, scroll_7
%%   Variables : x (a person), r (a record)
%%   Predicates:
%%     archivist(x)      x is an archivist
%%     verifies(x,r)     x verifies record r
%%     trusted(x)        x is trusted
%%     has_access(x,r)   x has access to record r
%%     sealed(r)         r is sealed
%%     permitted(x,r)    x is PERMITTED to inspect r
%%     inspects(x,r)     x actually inspects r
%%     finds_mark(x,r)   x finds a hidden mark in r
%%     stored(r)         r is stored in the royal archive
%%
%% Notation used for formulas:
%%   forall(V,F)  exists(V,F)  imp(A,B)  and(A,B)  or(A,B)  not(A)

:- discontiguous step1/2, step3/2, step4/2, step5/2, step6/2.
:- op(700, xfx, ===>).

%% ----------------------------------------------------------------------
%% Step 1: Predicate logic representation (Task 1)
%% ----------------------------------------------------------------------
step1(r1, forall(x, forall(r, imp(and(archivist(x), verifies(x,r)), trusted(x))))).
step1(r2, forall(x, forall(r, imp(and(trusted(x), and(has_access(x,r), sealed(r))), permitted(x,r))))).
step1(r3, forall(x, forall(r, imp(and(inspects(x,r), finds_mark(x,r)), verifies(x,r))))).
step1(r4, forall(r, imp(stored(r), sealed(r)))).
step1(f5, stored(scroll_7)).
step1(f6, has_access(mira, scroll_7)).
step1(f7, and(inspects(mira, scroll_7), finds_mark(mira, scroll_7))).
step1(f8, archivist(mira)).

%% ----------------------------------------------------------------------
%% Step 2: Negation of goal (Task 2)
%% Goal: trusted(mira).   Negated goal: not trusted(mira)
%% ----------------------------------------------------------------------
goal(trusted(mira)).
step2(ng, not(trusted(mira))).

%% ----------------------------------------------------------------------
%% Step 3: Implication removal   (A -> B  ==  ~A v B)
%% ----------------------------------------------------------------------
step3(r1, forall(x, forall(r, or(not(and(archivist(x), verifies(x,r))), trusted(x))))).
step3(r2, forall(x, forall(r, or(not(and(trusted(x), and(has_access(x,r), sealed(r)))), permitted(x,r))))).
step3(r3, forall(x, forall(r, or(not(and(inspects(x,r), finds_mark(x,r))), verifies(x,r))))).
step3(r4, forall(r, or(not(stored(r)), sealed(r)))).
step3(f5, stored(scroll_7)).
step3(f6, has_access(mira, scroll_7)).
step3(f7, and(inspects(mira, scroll_7), finds_mark(mira, scroll_7))).
step3(f8, archivist(mira)).
step3(ng, not(trusted(mira))).

%% ----------------------------------------------------------------------
%% Step 4: Move negation inward (De Morgan:  ~(A ^ B) == ~A v ~B)
%% ----------------------------------------------------------------------
step4(r1, forall(x, forall(r, or(or(not(archivist(x)), not(verifies(x,r))), trusted(x))))).
step4(r2, forall(x, forall(r, or(or(not(trusted(x)), or(not(has_access(x,r)), not(sealed(r)))), permitted(x,r))))).
step4(r3, forall(x, forall(r, or(or(not(inspects(x,r)), not(finds_mark(x,r))), verifies(x,r))))).
step4(r4, forall(r, or(not(stored(r)), sealed(r)))).
step4(f5, stored(scroll_7)).
step4(f6, has_access(mira, scroll_7)).
step4(f7, and(inspects(mira, scroll_7), finds_mark(mira, scroll_7))).
step4(f8, archivist(mira)).
step4(ng, not(trusted(mira))).

%% ----------------------------------------------------------------------
%% Step 5: Standardize variables apart (each rule gets its own variables)
%% ----------------------------------------------------------------------
step5(r1, forall(x1, forall(r1v, or(or(not(archivist(x1)), not(verifies(x1,r1v))), trusted(x1))))).
step5(r2, forall(x2, forall(r2v, or(or(not(trusted(x2)), or(not(has_access(x2,r2v)), not(sealed(r2v)))), permitted(x2,r2v))))).
step5(r3, forall(x3, forall(r3v, or(or(not(inspects(x3,r3v)), not(finds_mark(x3,r3v))), verifies(x3,r3v))))).
step5(r4, forall(r4v, or(not(stored(r4v)), sealed(r4v)))).
step5(f5, stored(scroll_7)).
step5(f6, has_access(mira, scroll_7)).
step5(f7, and(inspects(mira, scroll_7), finds_mark(mira, scroll_7))).
step5(f8, archivist(mira)).
step5(ng, not(trusted(mira))).

%% ----------------------------------------------------------------------
%% Step 6: Skolemization
%% There is NO existential quantifier in any formula, so no Skolem
%% constants/functions are needed.  We only drop the universal quantifiers.
%% ----------------------------------------------------------------------
step6(r1, or(or(not(archivist(X1)), not(verifies(X1,R1))), trusted(X1))).
step6(r2, or(or(not(trusted(X2)), or(not(has_access(X2,R2)), not(sealed(R2)))), permitted(X2,R2))).
step6(r3, or(or(not(inspects(X3,R3)), not(finds_mark(X3,R3))), verifies(X3,R3))).
step6(r4, or(not(stored(R4)), sealed(R4))).
step6(f5, stored(scroll_7)).
step6(f6, has_access(mira, scroll_7)).
step6(f7, and(inspects(mira, scroll_7), finds_mark(mira, scroll_7))).
step6(f8, archivist(mira)).
step6(ng, not(trusted(mira))).

%% ----------------------------------------------------------------------
%% Step 7: CNF clauses  (clause = list of literals; negative = neg(P))
%% Fact 7 is a conjunction, so it splits into TWO unit clauses.
%% Prolog variables (X1, R1, ...) are the clause variables.
%% ----------------------------------------------------------------------
kb(c1, [neg(archivist(X1)), neg(verifies(X1,R1)), trusted(X1)]).
kb(c2, [neg(trusted(X2)), neg(has_access(X2,R2)), neg(sealed(R2)), permitted(X2,R2)]).
kb(c3, [neg(inspects(X3,R3)), neg(finds_mark(X3,R3)), verifies(X3,R3)]).
kb(c4, [neg(stored(R4)), sealed(R4)]).
kb(c5, [stored(scroll_7)]).
kb(c6, [has_access(mira, scroll_7)]).
kb(c7a, [inspects(mira, scroll_7)]).
kb(c7b, [finds_mark(mira, scroll_7)]).
kb(c8, [archivist(mira)]).
negated_goal(ng, [neg(trusted(mira))]).

%% ----------------------------------------------------------------------
%% Step 8: Resolution implementation and execution
%% Strategy: set-of-support / linear resolution. Start from the negated
%% goal and repeatedly resolve the CURRENT clause with a (renamed) KB clause.
%% ----------------------------------------------------------------------

% complement(L, M): M is the literal that can clash with L
complement(neg(A), A) :- !.
complement(A, neg(A)).

% resolve_step(+Current, -Used, -UsedClauseRenamed, -Resolvent)
resolve_step(Cur, Name, Other, Res) :-
    kb(Name, C0),
    copy_term(C0, Other),                 % rename variables apart
    select(L, Cur, RestCur),
    complement(L, M),
    select(M, Other, RestOther),          % unification happens here
    append(RestCur, RestOther, R0),
    list_to_set(R0, Res).

% refute(+Clause, +StepNo, +Depth)
refute([], _, _) :- !,
    nl, writeln('>>> EMPTY CLAUSE [] DERIVED  --  contradiction found.').
refute(Cur, N, D) :-
    D > 0,
    resolve_step(Cur, Name, Other, Res),
    show(Cur, Name, Other, Res, N),
    N1 is N + 1, D1 is D - 1,
    refute(Res, N1, D1).

show(Cur, Name, Other, Res, N) :-
    format("Step ~w: resolve ~w~n", [N, Cur]),
    format("        with ~w ~w~n", [Name, Other]),
    format("        ==>  ~w~n~n", [Res]).

% helpers for printing the transformation stages
show_stage(Title, Pred) :-
    format("~n--- ~w ---~n", [Title]),
    forall(call(Pred, Id, F), format("  ~w: ~q~n", [Id, F])).

show_clauses :-
    format("~n--- Step 7: CNF clauses ---~n"),
    forall(kb(N, C), format("  ~w: ~q~n", [N, C])),
    negated_goal(NG, NC), format("  ~w: ~q~n", [NG, NC]).

main :-
    writeln('=== ROYAL ARCHIVE: RESOLUTION REFUTATION ==='),
    show_stage('Step 1: Predicate logic', step1),
    goal(G), format("~n--- Step 2: Negation of goal ---~n  Goal: ~q~n", [G]),
    forall(step2(I2, F2), format("  ~w: ~q~n", [I2, F2])),
    show_stage('Step 3: Implication removal', step3),
    show_stage('Step 4: Negation moved inward', step4),
    show_stage('Step 5: Standardized apart', step5),
    format("~n--- Step 6: Skolemization (none needed: no existentials) ---~n"),
    forall(step6(I6, F6), format("  ~w: ~q~n", [I6, F6])),
    show_clauses,
    format("~n--- Step 8: Resolution refutation ---~n~n"),
    negated_goal(_, Start),
    format("Start with negated goal: ~w~n~n", [Start]),
    (   refute(Start, 1, 10)
    ->  writeln('PROOF COMPLETE: trusted(mira) follows from the KB.'),
        writeln('(Clauses c2 and c4 were not needed.)')
    ;   writeln('No refutation found.')
    ).
