package instructions;

import builders.ForBuilder;
import declarations.VariableDeclaration;
import expressions.Expression;
import expressions.Literal;
import java.util.*;

public class For extends Block {
	private Expression expression;

	public For(ForBuilder forBuilder) throws Exception {
		this(
			forBuilder.getIteratingVariableName(),
			forBuilder.getExpression(),
			forBuilder.getInstructions()
		);
	}

	private For(
		char name,
		Expression expr,
		List<Instruction> instructions) throws Exception {

		super(
			List.of(new VariableDeclaration(name, new Literal(0))),
			instructions
		);

		this.expression = expr;
	}

	public void run(Block[] blocks) throws Exception {
		variableDeclarations = new VariableDeclaration[variableDeclarationsOriginal.length];
		for (int i = 0; i < variableDeclarations.length; i++) {
			variableDeclarations[i] = variableDeclarationsOriginal[i].deepCopy();
		}

		variableDeclarations[0].setInitialized(true);

		Block[] biggerBlocks = Arrays.copyOf(blocks, blocks.length + 1);
		biggerBlocks[biggerBlocks.length - 1] = this;

		if (Instruction.stepsCounter == 0) {
			System.out.print("Next instruction: calculating expression of ");
			System.out.println(this);
			debuggerStop(biggerBlocks);
		}

		int expressionValue;

		try {
			expressionValue = expression.evaluate(blocks);
		}
		catch (Exception e) {
			System.out.print("Failed to calculate the expression of ");
			System.out.print(this);
			System.out.println("; " + e.getMessage());
			System.out.println("All variables visible in the block containing the instruction:");
			for (int i = blocks.length - 1; i >= 0; i--) {
				blocks[i].printEvaluation(blocks);
			}
			throw new Exception("End of program");
		}

		Instruction.stepsCounter--; // calculating expression/entering for

		for (int i = 0; i < expressionValue; i++) {
			variableDeclarations[0].setValue(i);

			for (Instruction instruction : instructions) {
				instruction.run(biggerBlocks);
			}

			if (Instruction.stepsCounter == 0) {
				System.out.println("Next instruction: increasing iterating variable");
				debuggerStop(biggerBlocks);
			}

			this.variableDeclarations[0].setValue(
				this.variableDeclarations[0].getExpressionValue(biggerBlocks) + 1
			);

			Instruction.stepsCounter--; // increasing iterating variable, we treat an iteration as an instruction
		}
	}

	@Override
	public String toString() {
		return "for (int " + variableDeclarationsOriginal[0].getName() + " = 0; "
			+ variableDeclarationsOriginal[0].getName() + " < " + expression.toString() + "; "
			+ variableDeclarationsOriginal[0].getName() + "++)";
	}

	@Override
	public For deepCopy() throws Exception {
		For res = (For) super.deepCopy();

		res.expression = expression.deepCopy();

		return res;
	}
}
