package instructions;

import expressions.Expression;

public class Print extends Instruction {
	private Expression expression;

	private Print() {}

	public Print(Expression expression) {
		this.expression = expression;
	}

	public void run(Block[] blocks) throws Exception {
		if (Instruction.stepsCounter == 0) {
			System.out.println("Next instruction: " + this);
			debuggerStop(blocks);
		}

		try {
			System.out.println(expression.evaluate(blocks));
		}
		catch (Exception e) {
			System.out.print("FROM PRINT: Failed to calculate the expression of " + this);
			System.out.println("; " + e.getMessage());
			System.out.println("All variables visible in the block containing the instruction:");
			for (int j = blocks.length - 1; j >= 0; j--) {
				blocks[j].printEvaluation(blocks);
			}
			throw new Exception("End of program");
		}

		Instruction.stepsCounter--; // printing
	}

	@Override
	public String toString() {
		return "print " + expression.toString();
	}

	@Override
	public Print deepCopy() throws Exception {
		Print res = new Print();

		res.expression = expression.deepCopy();

		return res;
	}
}
