% ============================================================
% THE GREEK PHILOSOPHER PROBLEM
% Resolution Refutation in First-Order Predicate Logic
% ============================================================


% ============================================================
% STEP 1: PREDICATE LOGIC REPRESENTATION
% ============================================================
%
% All men are mortal:
%       forall(X, man(X) -> mortal(X))
%
% All Greeks are men:
%       forall(X, greek(X) -> man(X))
%
% Socrates is a Greek:
%       greek(socrates)
%
% Goal:
%       mortal(socrates)
%
% Here 'v' is used as a symbolic FOL variable name.
% It is an atom, not a Prolog variable.


predicate_logic([
    forall(v, implies(man(v), mortal(v))),
    forall(v, implies(greek(v), man(v))),
    greek(socrates)
]).

goal(mortal(socrates)).


step1 :-
    writeln('=============================================='),
    writeln('STEP 1: PREDICATE LOGIC REPRESENTATION'),
    writeln('=============================================='),

    predicate_logic(Facts),

    forall(
        member(Fact, Facts),
        (
            write('  '),
            writeln(Fact)
        )
    ),

    write('  Goal: '),
    writeln(mortal(socrates)),
    nl.


% ============================================================
% STEP 2: NEGATION OF THE GOAL
% ============================================================
%
% For resolution refutation, negate the goal:
%
% Goal:
%       mortal(socrates)
%
% Negated goal:
%       not(mortal(socrates))


negated_goal(not(mortal(socrates))).


step2 :-
    writeln('=============================================='),
    writeln('STEP 2: NEGATION OF GOAL'),
    writeln('=============================================='),

    goal(Goal),
    negated_goal(NegatedGoal),

    write('  Original goal: '),
    writeln(Goal),

    write('  Negated goal:  '),
    writeln(NegatedGoal),

    nl.


% ============================================================
% STEP 3: IMPLICATION REMOVAL
% ============================================================
%
% Rule:
%
%       A -> B
%
% becomes:
%
%       not(A) OR B
%
%
% Therefore:
%
% forall(v, implies(man(v), mortal(v)))
%
% becomes:
%
% forall(v, or(not(man(v)), mortal(v)))
%
%
% And:
%
% forall(v, implies(greek(v), man(v)))
%
% becomes:
%
% forall(v, or(not(greek(v)), man(v)))


remove_implication(
    forall(V, implies(A, B)),
    forall(V, or(not(A), B))
) :- !.

remove_implication(
    Formula,
    Formula
).


step3 :-
    writeln('=============================================='),
    writeln('STEP 3: IMPLICATION REMOVAL'),
    writeln('=============================================='),

    predicate_logic(Facts),

    forall(
        member(Fact, Facts),
        (
            remove_implication(Fact, Result),

            write('  '),
            write(Fact),
            write('  ==>  '),
            writeln(Result)
        )
    ),

    negated_goal(NegatedGoal),

    write('  Negated goal remains: '),
    writeln(NegatedGoal),

    nl.


% ============================================================
% STEP 4: NEGATION MOVEMENT
% ============================================================
%
% Rules implemented:
%
% not(not(A)) -> A
%
% not(A AND B) -> not(A) OR not(B)
%
% not(A OR B) -> not(A) AND not(B)
%
% In this particular problem, all negations are already directly
% applied to atomic predicates, so no further movement is needed.


move_negation(
    not(not(A)),
    A
) :- !.


move_negation(
    not(and(A, B)),
    or(not(A), not(B))
) :- !.


move_negation(
    not(or(A, B)),
    and(not(A), not(B))
) :- !.


move_negation(
    forall(V, Body),
    forall(V, NewBody)
) :-
    move_negation(Body, NewBody),
    !.


move_negation(
    exists(V, Body),
    exists(V, NewBody)
) :-
    move_negation(Body, NewBody),
    !.


move_negation(
    Formula,
    Formula
).


step4 :-
    writeln('=============================================='),
    writeln('STEP 4: NEGATION MOVEMENT'),
    writeln('=============================================='),

    predicate_logic(Facts),

    forall(
        member(Fact, Facts),
        (
            remove_implication(Fact, NoImplication),

            move_negation(
                NoImplication,
                Result
            ),

            write('  '),
            write(NoImplication),
            write('  ==>  '),
            writeln(Result)
        )
    ),

    negated_goal(NegatedGoal),

    move_negation(
        NegatedGoal,
        NegatedResult
    ),

    write('  Negated goal: '),
    writeln(NegatedResult),

    nl.


% ============================================================
% STEP 5: STANDARDIZATION APART
% ============================================================
%
% The same FOL variable name 'v' occurs in both universal
% statements.
%
% We standardize them apart:
%
% First clause:
%       v -> v1
%
% Second clause:
%       v -> v2
%
% v1 and v2 are symbolic FOL variable names.
% They are intentionally represented as lowercase atoms because
% this stage is showing the FOL transformation.
%
% Actual Prolog variables are introduced in STEP 7 for the
% resolution engine.


standardize_apart(
    forall(v, Body),
    v1,
    forall(v1, NewBody)
) :-
    replace_fol_variable(
        Body,
        v,
        v1,
        NewBody
).


standardize_apart(
    forall(v, Body),
    v2,
    forall(v2, NewBody)
) :-
    replace_fol_variable(
        Body,
        v,
        v2,
        NewBody
).


replace_fol_variable(
    Term,
    Old,
    New,
    Result
) :-
    replace_term(
        Term,
        Old,
        New,
        Result
    ).


replace_term(
    Term,
    Old,
    New,
    New
) :-
    Term == Old,
    !.


replace_term(
    Term,
    Old,
    New,
    Result
) :-
    compound(Term),
    !,

    Term =.. [Functor | Arguments],

    replace_arguments(
        Arguments,
        Old,
        New,
        NewArguments
    ),

    Result =.. [Functor | NewArguments].


replace_term(
    Term,
    _,
    _,
    Term
).


replace_arguments(
    [],
    _,
    _,
    []
).


replace_arguments(
    [H | T],
    Old,
    New,
    [H2 | T2]
) :-
    replace_term(H, Old, New, H2),
    replace_arguments(T, Old, New, T2).


step5 :-
    writeln('=============================================='),
    writeln('STEP 5: STANDARDIZATION APART'),
    writeln('=============================================='),

    predicate_logic(Facts),

    Facts = [
        FirstClause,
        SecondClause,
        Fact
    ],

    standardize_apart(
        FirstClause,
        v1,
        StandardFirst
    ),

    standardize_apart(
        SecondClause,
        v2,
        StandardSecond
    ),

    write('  Original clause 1:     '),
    writeln(FirstClause),

    write('  Standardized clause 1: '),
    writeln(StandardFirst),

    write('  Original clause 2:     '),
    writeln(SecondClause),

    write('  Standardized clause 2: '),
    writeln(StandardSecond),

    write('  Fact:                  '),
    writeln(Fact),

    negated_goal(NegatedGoal),

    write('  Negated goal:          '),
    writeln(NegatedGoal),

    nl.


% ============================================================
% STEP 6: SKOLEMIZATION
% ============================================================
%
% Skolemization is required only when existential quantifiers
% are present.
%
% This problem contains no existential quantifier.
%
% Therefore:
%
%       No Skolem function or Skolem constant is required.
%
% The formulas remain unchanged.


skolemize(
    Formula,
    Formula
).


step6 :-
    writeln('=============================================='),
    writeln('STEP 6: SKOLEMIZATION'),
    writeln('=============================================='),

    writeln('  No existential quantifiers are present.'),

    writeln('  Therefore, no Skolem function or'),
    writeln('  Skolem constant is required.'),

    predicate_logic(Facts),

    forall(
        member(Fact, Facts),
        (
            remove_implication(
                Fact,
                NoImplication
            ),

            move_negation(
                NoImplication,
                NegationMoved
            ),

            skolemize(
                NegationMoved,
                Result
            ),

            write('  '),
            writeln(Result)
        )
    ),

    negated_goal(NegatedGoal),

    write('  Negated goal: '),
    writeln(NegatedGoal),

    nl.


% ============================================================
% STEP 7: CNF CLAUSES
% ============================================================
%
% After implication removal and negation movement, the clauses
% are already in CNF.
%
% C1:
%       not(man(X)) OR mortal(X)
%
% C2:
%       not(greek(X)) OR man(X)
%
% C3:
%       greek(socrates)
%
% C4:
%       not(mortal(socrates))
%
% Here X is an actual Prolog variable because these clauses are
% used by the resolution engine.


cnf_clause(
    1,
    [not(man(X)), mortal(X)]
).


cnf_clause(
    2,
    [not(greek(X)), man(X)]
).


cnf_clause(
    3,
    [greek(socrates)]
).


cnf_clause(
    4,
    [not(mortal(socrates))]
).


step7 :-
    writeln('=============================================='),
    writeln('STEP 7: CNF CLAUSES'),
    writeln('=============================================='),

    cnf_clause(1, C1),
    cnf_clause(2, C2),
    cnf_clause(3, C3),
    cnf_clause(4, C4),

    write('  C1 = '),
    writeln(C1),

    write('  C2 = '),
    writeln(C2),

    write('  C3 = '),
    writeln(C3),

    write('  C4 = '),
    writeln(C4),

    nl.


% ============================================================
% STEP 8: RESOLUTION IMPLEMENTATION
% ============================================================


% Two literals are complementary if one is the negation
% of the other.


complementary(
    A,
    not(A)
).


complementary(
    not(A),
    A
).


% ------------------------------------------------------------
% Resolution algorithm
%
% 1. Copy both clauses.
% 2. Select complementary literals.
% 3. Remove the complementary literals.
% 4. Combine the remaining literals.
% 5. Remove duplicate literals.
%
% copy_term/2 is used so that the original clauses are not
% permanently instantiated during resolution.
% ------------------------------------------------------------


resolve(
    C1,
    C2,
    Resolvent
) :-

    copy_term(
        C1,
        C1Copy
    ),

    copy_term(
        C2,
        C2Copy
    ),

    select(
        L1,
        C1Copy,
        Rest1
    ),

    select(
        L2,
        C2Copy,
        Rest2
    ),

    complementary(
        L1,
        L2
    ),

    append(
        Rest1,
        Rest2,
        Combined
    ),

    sort(
        Combined,
        Resolvent
    ).


% ============================================================
% DISPLAY THE RESOLUTION REFUTATION
% ============================================================


step8 :-

    writeln('=============================================='),
    writeln('STEP 8: RESOLUTION REFUTATION'),
    writeln('=============================================='),

    % --------------------------------------------------------
    % Initial clauses
    % --------------------------------------------------------

    cnf_clause(1, C1),
    cnf_clause(2, C2),
    cnf_clause(3, C3),
    cnf_clause(4, C4),

    % --------------------------------------------------------
    % Resolution Step 1
    %
    % C1:
    % not(man(X)) OR mortal(X)
    %
    % C4:
    % not(mortal(socrates))
    %
    % Result:
    % not(man(socrates))
    % --------------------------------------------------------

    writeln('Resolution Step 1:'),

    write('  C1 = '),
    writeln(C1),

    write('  C4 = '),
    writeln(C4),

    resolve(
        C1,
        C4,
        R1
    ),

    write('  R1 = '),
    writeln(R1),

    nl,

    % --------------------------------------------------------
    % Resolution Step 2
    %
    % C2:
    % not(greek(X)) OR man(X)
    %
    % R1:
    % not(man(socrates))
    %
    % Result:
    % not(greek(socrates))
    % --------------------------------------------------------

    writeln('Resolution Step 2:'),

    write('  C2 = '),
    writeln(C2),

    write('  R1 = '),
    writeln(R1),

    resolve(
        C2,
        R1,
        R2
    ),

    write('  R2 = '),
    writeln(R2),

    nl,

    % --------------------------------------------------------
    % Resolution Step 3
    %
    % C3:
    % greek(socrates)
    %
    % R2:
    % not(greek(socrates))
    %
    % Result:
    % []
    %
    % [] is the empty clause.
    % --------------------------------------------------------

    writeln('Resolution Step 3:'),

    write('  C3 = '),
    writeln(C3),

    write('  R2 = '),
    writeln(R2),

    resolve(
        C3,
        R2,
        R3
    ),

    write('  R3 = '),
    writeln(R3),

    nl,

    % --------------------------------------------------------
    % Final proof
    % --------------------------------------------------------

    R3 = [],

    writeln('=============================================='),
    writeln('EMPTY CLAUSE DERIVED: []'),
    writeln('=============================================='),

    writeln('The negated goal causes a contradiction.'),

    writeln('Therefore, Socrates is mortal.'),

    nl.


% ============================================================
% RUN ALL EIGHT STEPS
% ============================================================


all_steps :-
    step1,
    step2,
    step3,
    step4,
    step5,
    step6,
    step7,
    step8.
