/*
   The Philosopher and Scholars Problem
   Resolution Refutation using First-Order Predicate Logic

   Given:
      All philosophers are thinkers.
      All thinkers are scholars.
      All scholars are knowledgeable.
      Aristotle is a philosopher.

   Goal:
      Prove that Aristotle is knowledgeable.
*/


% ================================================================
% STEP 1: PREDICATE LOGIC REPRESENTATION
% ================================================================

predicate_logic :-
    nl,
    writeln('--------------------------------------------------'),
    writeln('STEP 1: PREDICATE LOGIC REPRESENTATION'),
    writeln('--------------------------------------------------'),
    writeln('forall X (philosopher(X) -> thinker(X))'),
    writeln('forall X (thinker(X) -> scholar(X))'),
    writeln('forall X (scholar(X) -> knowledgeable(X))'),
    writeln('philosopher(aristotle)'),
    writeln('Goal: knowledgeable(aristotle)').


% ================================================================
% STEP 2: NEGATE THE GOAL
% ================================================================

negate_goal :-
    nl,
    writeln('--------------------------------------------------'),
    writeln('STEP 2: NEGATION OF THE GOAL'),
    writeln('--------------------------------------------------'),
    writeln('Goal: knowledgeable(aristotle)'),
    writeln('For resolution refutation, add:'),
    writeln('not(knowledgeable(aristotle))').


% ================================================================
% STEP 3: REMOVE IMPLICATIONS
% ================================================================

remove_implications :-
    nl,
    writeln('--------------------------------------------------'),
    writeln('STEP 3: IMPLICATION REMOVAL'),
    writeln('--------------------------------------------------'),
    writeln('philosopher(X) -> thinker(X)'),
    writeln('      becomes not(philosopher(X)) OR thinker(X)'),
    writeln(''),
    writeln('thinker(X) -> scholar(X)'),
    writeln('      becomes not(thinker(X)) OR scholar(X)'),
    writeln(''),
    writeln('scholar(X) -> knowledgeable(X)'),
    writeln('      becomes not(scholar(X)) OR knowledgeable(X)').


% ================================================================
% STEP 4: MOVE NEGATIONS
% ================================================================

move_negations :-
    nl,
    writeln('--------------------------------------------------'),
    writeln('STEP 4: NEGATION MOVEMENT'),
    writeln('--------------------------------------------------'),
    writeln('There are no nested negations in the formulas.'),
    writeln('Therefore no further negation movement is required.'),
    writeln('The negated goal is:'),
    writeln('not(knowledgeable(aristotle))').


% ================================================================
% STEP 5: STANDARDIZATION APART
% ================================================================

standardize :-
    nl,
    writeln('--------------------------------------------------'),
    writeln('STEP 5: STANDARDIZATION APART'),
    writeln('--------------------------------------------------'),
    writeln('C1: not(philosopher(P)) OR thinker(P)'),
    writeln('C2: not(thinker(T)) OR scholar(T)'),
    writeln('C3: not(scholar(S)) OR knowledgeable(S)'),
    writeln('C4: philosopher(aristotle)'),
    writeln('C5: not(knowledgeable(aristotle))'),
    writeln(''),
    writeln('Different variables are used in the three universal clauses.').


% ================================================================
% STEP 6: SKOLEMIZATION
% ================================================================

skolemize :-
    nl,
    writeln('--------------------------------------------------'),
    writeln('STEP 6: SKOLEMIZATION'),
    writeln('--------------------------------------------------'),
    writeln('No existential quantifier occurs in the problem.'),
    writeln('Therefore no Skolem function or Skolem constant is introduced.'),
    writeln('The universal clauses remain unchanged.').


% ================================================================
% STEP 7: CONJUNCTIVE NORMAL FORM
% ================================================================

make_cnf([
    [neg(philosopher(P)), thinker(P)],
    [neg(thinker(T)), scholar(T)],
    [neg(scholar(S)), knowledgeable(S)],
    [philosopher(aristotle)],
    [neg(knowledgeable(aristotle))]
]) :-
    nl,
    writeln('--------------------------------------------------'),
    writeln('STEP 7: CNF CLAUSES'),
    writeln('--------------------------------------------------'),
    writeln('C1 = { neg(philosopher(P)), thinker(P) }'),
    writeln('C2 = { neg(thinker(T)), scholar(T) }'),
    writeln('C3 = { neg(scholar(S)), knowledgeable(S) }'),
    writeln('C4 = { philosopher(aristotle) }'),
    writeln('C5 = { neg(knowledgeable(aristotle)) }').


% ================================================================
% RESOLUTION OPERATIONS
% ================================================================

% Two literals are complementary when one is positive
% and the other is its negation.

opposite(neg(X), X).
opposite(X, neg(X)).


% Resolve two clauses.

resolution(Left, Right, Result) :-
    copy_term((Left, Right), (L, R)),
    member(A, L),
    member(B, R),
    opposite(A, B),
    delete(L, A, L1),
    delete(R, B, R1),
    append(L1, R1, Temp),
    sort(Temp, Result).


% Display a clause in a readable form.

display_clause([]) :-
    write('EMPTY CLAUSE').

display_clause([X]) :-
    write(X).

display_clause([X|Xs]) :-
    write(X),
    write(' OR '),
    display_clause(Xs).


% Display one resolution step.

show_step(Number, FirstName, First, SecondName, Second, Result) :-
    nl,
    format('Resolution ~w~n', [Number]),
    write(FirstName),
    write(' : '),
    display_clause(First),
    nl,
    write(SecondName),
    write(' : '),
    display_clause(Second),
    nl,
    write('Result : '),
    display_clause(Result),
    nl.


% ================================================================
% STEP 8: RESOLUTION REFUTATION
% ================================================================

prove(Clauses) :-
    Clauses = [C1, C2, C3, C4, C5],

    nl,
    writeln('--------------------------------------------------'),
    writeln('STEP 8: RESOLUTION REFUTATION'),
    writeln('--------------------------------------------------'),

    % Philosopher -> Thinker
    resolution(C1, C4, R1),
    show_step(1, 'C1', C1, 'C4', C4, R1),

    % Thinker -> Scholar
    resolution(C2, R1, R2),
    show_step(2, 'C2', C2, 'R1', R1, R2),

    % Scholar -> Knowledgeable
    resolution(C3, R2, R3),
    show_step(3, 'C3', C3, 'R2', R2, R3),

    % Knowledgeable and negated goal
    resolution(C5, R3, R4),
    show_step(4, 'C5', C5, 'R3', R3, R4),

    R4 = [],

    nl,
    writeln('--------------------------------------------------'),
    writeln('RESULT'),
    writeln('--------------------------------------------------'),
    writeln('Empty clause derived: []'),
    writeln('The negated goal leads to a contradiction.'),
    writeln('Therefore knowledgeable(aristotle) is proved.'),
    writeln('--------------------------------------------------').


% ================================================================
% COMPLETE ASSIGNMENT
% ================================================================

main :-
    predicate_logic,
    negate_goal,
    remove_implications,
    move_negations,
    standardize,
    skolemize,
    make_cnf(Clauses),
    prove(Clauses).
