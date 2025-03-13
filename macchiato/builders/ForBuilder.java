package builders;

import expressions.Expression;
import instructions.Assignment;
import instructions.Block;
import instructions.Instruction;
import instructions.If;
import instructions.For;
import instructions.Print;
import instructions.ProcedureCall;

import java.util.*;

public class ForBuilder {

	private char iteratingVariableName;

	private Expression expression;

	private List<Instruction> instructions;

	public char getIteratingVariableName() {
		return iteratingVariableName;
	}

	public Expression getExpression() {
		return expression;
	}

	public List<Instruction> getInstructions() {
		return instructions;
	}

	public ForBuilder(char iteratingVariableName, Expression expression) {
		this.iteratingVariableName = iteratingVariableName;
		this.expression = expression;
		this.instructions = new ArrayList<>();
	}

	public ForBuilder assign(char name, Expression expression) {
		instructions.add(new Assignment(name, expression));
		return this;
	}

	public ForBuilder block (Block block) {
		instructions.add(block);
		return this;
	}

	public ForBuilder forLoop(For forLoop) {
		instructions.add(forLoop);
		return this;
	}

	public ForBuilder ifElse (If ifElse) {
		instructions.add(ifElse);
		return this;
	}

	public ForBuilder invoke(String name, List<Expression> argumentsExpressions) {
		instructions.add(new ProcedureCall(name, argumentsExpressions));
		return this;
	}

	public ForBuilder print(Expression expression) {
		instructions.add(new Print(expression));
		return this;
	}

	public For build() throws Exception {
		return new For(this);
	}
}
