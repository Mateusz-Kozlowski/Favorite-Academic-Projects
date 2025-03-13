package instructions;

import builders.BlockBuilder;
import declarations.*;
import expressions.Expression;
import expressions.Literal;

import java.util.*;

public class ProcedureCall extends Instruction {
	private String procedureName;

	private Expression[] argumentsExpressions;

	private ProcedureCall() {}

	public ProcedureCall(String procedureName, List<Expression> argumentsExpressions) {
		this.procedureName = procedureName;

		if (argumentsExpressions == null) {
			this.argumentsExpressions = null;
		}
		else {
			this.argumentsExpressions = new Expression[argumentsExpressions.size()];

			ListIterator<Expression> expressionListIterator = argumentsExpressions.listIterator();

			for (int i = 0; i < argumentsExpressions.size(); i++) {
				this.argumentsExpressions[i] = expressionListIterator.next();
			}
		}
	}

	@Override
	protected void run(Block[] blocks) throws Exception {
		// First debugger stuff.
		if (Instruction.stepsCounter == 0) {
			System.out.print("Next instruction: calculating expression of ");
			System.out.println(this);
			debuggerStop(blocks);
		}

		Instruction.stepsCounter--;

		// Now we need to check if such a procedure even is available here.
		ProcedureDeclaration declaration = null;

		for (int i = blocks.length - 1; i >= 0; i--) {
			if (blocks[i].containsProcedure(procedureName)) {
				declaration = blocks[i].getProcedureDeclaration(procedureName);
				break;
			}
		}

		if (declaration == null) {
			throw new Exception("There is no procedure declaration named " + procedureName + " available here.");
		}

		// First expressions of arguments will be evaluated.
		// Then instructions will be run.
		if (argumentsExpressions == null || declaration.getArguments() == null) {
			throw new Exception("Arguments or arguments expressions is null!");
		}

		if (argumentsExpressions.length != declaration.getArguments().size()) {
			throw new Exception("The number of arguments is different from the number of arguments expressions!");
		}

		VariableDeclaration[] evaluatedArguments = new VariableDeclaration[argumentsExpressions.length];

		for (int i = 0; i < argumentsExpressions.length; i++) {
			evaluatedArguments[i] = new VariableDeclaration(
				declaration.getArguments().get(i),
				new Literal(argumentsExpressions[i].evaluate(blocks))
			);
		}

		// Now it's time to run instructions.
		// But first we need to temporarily add arguments of the functions to blocks array.
		Block[] biggerBlocks = Arrays.copyOf(blocks, blocks.length + 1);

		//biggerBlocks[biggerBlocks.length - 1] = new instructions.Block(evaluatedArguments, new instructions.Instruction[]{});
		biggerBlocks[biggerBlocks.length - 1] = new BlockBuilder().declareVariables(evaluatedArguments).build();

		biggerBlocks[biggerBlocks.length - 1].calculateVariablesDeclarations(blocks);

		// Finally we can run instructions.
		declaration.getContentBlock().run(biggerBlocks);
	}

	@Override
	public String toString() {
		StringBuilder res = new StringBuilder("void ");
		res.append(procedureName);
		res.append('(');

		int i = 0;
		for (Expression argumentExpression : argumentsExpressions) {
			res.append(argumentExpression);

			if (i != argumentsExpressions.length - 1) {
				res.append(", ");
			}

			i++;
		}

		res.append(')');

		return res.toString();
	}

	@Override
	public ProcedureCall deepCopy() throws Exception {
		ProcedureCall res = new ProcedureCall();

		res.procedureName = procedureName;

		if (argumentsExpressions == null) {
			res.argumentsExpressions = null;
		}
		else {
			res.argumentsExpressions = new Expression[argumentsExpressions.length];
			for (int i = 0; i < argumentsExpressions.length; i++) {
				res.argumentsExpressions[i] = argumentsExpressions[i].deepCopy();
			}
		}

		return res;
	}
}
