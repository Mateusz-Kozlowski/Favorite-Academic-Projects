package instructions;

import builders.BlockBuilder;
import declarations.*;
import java.util.Arrays;
import java.util.Objects;
import java.util.*;

public class Block extends Instruction {
	protected VariableDeclaration[] variableDeclarations;

	protected VariableDeclaration[] variableDeclarationsOriginal;

	protected ProcedureDeclaration[] procedureDeclarations;

	protected Instruction[] instructions;

	public Block(BlockBuilder blockBuilder) throws Exception {
		this(
			blockBuilder.getVariableDeclarations(),
			blockBuilder.getProcedureDeclarations(),
			blockBuilder.getInstructions()
		);
	}

	private Block() {};

	protected Block(
		List<VariableDeclaration> variableDeclarations,
		List<Instruction> instructions) throws Exception {

		this(variableDeclarations, new ArrayList<>(), instructions);
	}

	private Block(
		List<VariableDeclaration> variableDeclarations,
		List<ProcedureDeclaration> procedureDeclarations,
		List<Instruction> instructions) throws Exception {

		assertNoVariablesDuplicates(variableDeclarations);
		assertNoProceduresDuplicates(procedureDeclarations);

		initVariableDeclarationsOriginal(variableDeclarations);
		initVariableDeclarations();

		initProcedureDeclarations(procedureDeclarations);

		initInstructions(instructions);
	}

	private void assertNoVariablesDuplicates(List<VariableDeclaration> variableDeclarations) throws Exception {
		for (VariableDeclaration variableDeclaration1 : variableDeclarations) {
			for (VariableDeclaration variableDeclaration2 : variableDeclarations) {
				if (Objects.equals(variableDeclaration1.getName(), variableDeclaration2.getName())) {
					if (variableDeclaration1 != variableDeclaration2) {
						throw new Exception("Cannot declare 2 variables with the same name in a block");
					}
				}
			}
		}
	}

	private void assertNoProceduresDuplicates(List<ProcedureDeclaration> procedureDeclarations) throws Exception {
		for (ProcedureDeclaration procedureDeclaration1 : procedureDeclarations) {
			for (ProcedureDeclaration procedureDeclaration2 : procedureDeclarations) {
				if (Objects.equals(procedureDeclaration1.getName(), procedureDeclaration2.getName())) {
					if (procedureDeclaration1 != procedureDeclaration2) {
						throw new Exception("Cannot declare 2 procedures with the same name in a block");
					}
				}
			}
		}
	}

	private void initVariableDeclarationsOriginal(List<VariableDeclaration> variableDeclarations) {
		variableDeclarationsOriginal = new VariableDeclaration[variableDeclarations.size()];

		ListIterator<VariableDeclaration> variableDeclarationListIterator = variableDeclarations.listIterator();

		for (int i = 0; i < variableDeclarations.size(); i++) {
			variableDeclarationsOriginal[i] = variableDeclarationListIterator.next();
		}
	}

	// Initializes variables declarations by performing a deep copy on variableDeclarationsOriginal array.
	private void initVariableDeclarations() {
		variableDeclarations = new VariableDeclaration[variableDeclarationsOriginal.length];

		for (int i = 0; i < variableDeclarations.length; i++) {
			variableDeclarations[i] = variableDeclarationsOriginal[i].deepCopy();
		}
	}

	private void initProcedureDeclarations(List<ProcedureDeclaration> procedureDeclarations) {
		this.procedureDeclarations = new ProcedureDeclaration[procedureDeclarations.size()];

		ListIterator<ProcedureDeclaration> procedureDeclarationIterator = procedureDeclarations.listIterator();

		for (int i = 0; i < this.procedureDeclarations.length; i++) {
			this.procedureDeclarations[i] = procedureDeclarationIterator.next();
		}
	}

	private void initInstructions(List<Instruction> instructions) {
		this.instructions = new Instruction[instructions.size()];

		ListIterator<Instruction> instructionListIterator = instructions.listIterator();

		for (int i = 0; i < this.instructions.length; i++) {
			this.instructions[i] = instructionListIterator.next();
		}
	}

	public int startProgram(boolean debug) {
		if (!debug) {
			Instruction.stepsCounter = -1;
		}
		else {
			Instruction.stepsCounter = 0;
		}

		try {
			this.run(new Block[]{});
		}
		catch (Exception e) {
			return -1;
		}

		printEvaluation(new Block[]{ this });

		return 0;
	}

	protected void run(Block[] blocks) throws Exception {
		variableDeclarations = new VariableDeclaration[variableDeclarationsOriginal.length];
		for (int i = 0; i < variableDeclarations.length; i++) {
			variableDeclarations[i] = variableDeclarationsOriginal[i].deepCopy();
		}

		if (Instruction.stepsCounter == 0) {
			System.out.println("Next instruction: entering a block");
			debuggerStop(blocks);
		}

		Instruction.stepsCounter--; // entering the block

		Block[] biggerBlocks = Arrays.copyOf(blocks, blocks.length + 1);
		biggerBlocks[biggerBlocks.length - 1] = this;

		calculateVariablesDeclarations(biggerBlocks);

		for (Instruction instruction : instructions) {
			instruction.run(biggerBlocks);
		}

		if (Instruction.stepsCounter == 0) {
			System.out.println("Next instruction: exiting a block");
			debuggerStop(biggerBlocks);
		}

		Instruction.stepsCounter--; // exiting the block
	}

	// Evaluates variables and sets their initialized boolean to true.
	protected void calculateVariablesDeclarations(Block[] biggerBlocks) throws Exception {
		for (VariableDeclaration variableDeclaration : variableDeclarations) {
			if (Instruction.stepsCounter == 0) {
				System.out.println("Next instruction: initialization " + variableDeclaration.toString().toString());
				debuggerStop(biggerBlocks);
			}

			if (variableAlreadyDeclared(variableDeclaration.getName())) {
				System.out.println("There is already variable named " + variableDeclaration.getName() + " in this block");
				System.out.println("The second one won't be declared");
			}
			else {
				try {
					variableDeclaration.setValue(variableDeclaration.getExpressionValue(biggerBlocks));
					variableDeclaration.setInitialized(true);
				}
				catch (Exception e) {
					variableDeclaration.setInitialized(false);

					System.out.print("Failed to calculate the expression of initialization: ");
					System.out.print(variableDeclaration.toString());
					System.out.println("; " + e.getMessage());
					System.out.println("All variables visible in the block containing the instruction:");
					for (int j = biggerBlocks.length - 1; j >= 0; j--) {
						biggerBlocks[j].printEvaluation(biggerBlocks);
					}
					throw new Exception("End of program");
				}
			}

			Instruction.stepsCounter--; // an initialization
		}
	}

	private boolean variableAlreadyDeclared(char name) {
		for (VariableDeclaration variableDeclaration : variableDeclarations) {
			if (variableDeclaration.getName() == name && variableDeclaration.isInitialized()) {
				return true;
			}
		}

		return false;
	}

	// Assumes that the block contains variable named name.
	// instructions.If that variable wasn't found throws an exception.
	public int getVariableValue(char name, Block[] blocks) throws Exception {
		for (VariableDeclaration variableDeclaration : variableDeclarations) {
			if (variableDeclaration.getName() == name && variableDeclaration.isInitialized()) {
				return variableDeclaration.getExpressionValue(blocks);
			}
		}

		throw new Exception("There is no initialized variable named " + name + " in this block");
	}

	// Assumes that the block contains procedure declaration named name.
	// instructions.If that declaration wasn't found throws an exception.
	protected ProcedureDeclaration getProcedureDeclaration(String name) throws Exception {
		for (ProcedureDeclaration procedureDeclaration : procedureDeclarations) {
			if (Objects.equals(procedureDeclaration.getName(), name)) {
				return procedureDeclaration;
			}
		}

		throw new Exception("There is no procedure declaration named " + name + " in this block");
	}

	// assumes that the block contains variable named name
	protected void assign(char name, int value) throws Exception {
		for (VariableDeclaration variableDeclaration : variableDeclarations) {
			if (variableDeclaration.getName() == name && variableDeclaration.isInitialized()) {
				variableDeclaration.setValue(value);
				return;
			}
		}

		throw new Exception( "There is no variable named " + name + " in this block");
	}

	public boolean containsInitializedVariable(char name) {
		for (VariableDeclaration variableDeclaration : variableDeclarations) {
			if(variableDeclaration.getName() == name && variableDeclaration.isInitialized()) {
				return true;
			}
		}

		return false;
	}

	protected boolean containsProcedure(String name) {
		for (ProcedureDeclaration procedureDeclaration : procedureDeclarations) {
			if (Objects.equals(procedureDeclaration.getName(), name)) {
				return true;
			}
		}

		return false;
	}

	protected void printEvaluation(Block[] metaBlocks) {
		for (VariableDeclaration variableDeclaration : variableDeclarations) {
			if (variableDeclaration.isInitialized()) {
				try {
					System.out.println(
						variableDeclaration.getName() + " = " + variableDeclaration.getExpressionValue(metaBlocks)
					);
				}
				catch (Exception e) {
					System.out.println(variableDeclaration.getName() + " evaluation failed (" + e.getMessage() + ')');
				}
			}
			else {
				System.out.println(variableDeclaration.getName() + " not initialized");
			}
		}
	}

	protected String getMemoryDump(Block[] metaBlocks) {
		StringBuilder res = new StringBuilder();

		res.append("Visible procedures:\n");

		for (Block block : metaBlocks) {
			for (ProcedureDeclaration procedureDeclaration : block.procedureDeclarations) {
				res.append(procedureDeclaration.getString()).append('\n');
			}
		}

		res.append("d0:\n");

		for (VariableDeclaration variableDeclaration : variableDeclarations) {
			if (variableDeclaration.isInitialized()) {
				try {
					res.append(variableDeclaration.getName()).append(" = ");
					res.append(variableDeclaration.getExpressionValue(metaBlocks)).append('\n');
				}
				catch (Exception e) {
					res.append(variableDeclaration.getName()).append(" evaluation failed (");
					res.append(e.getMessage()).append(')').append('\n');
				}
			}
			else {
				res.append(variableDeclaration.getName()).append(" not initialized").append('\n');
			}
		}

		return res.toString();
	}

	@Override
	public String toString() {
		StringBuilder res = new StringBuilder();

		for (VariableDeclaration originalVariableDeclaration : variableDeclarationsOriginal) {
			res.append(originalVariableDeclaration.toString()).append('\n');
		}

		for (Instruction instruction : instructions) {
			res.append(instruction.toString()).append('\n');
		}

		return res.toString();
	}

	@Override
	public Block deepCopy() throws Exception {
		Block res = new Block();

		if (variableDeclarations == null) {
			res.variableDeclarations = null;
		}
		else {
			res.variableDeclarations = new VariableDeclaration[variableDeclarations.length];
			for(int i = 0; i < res.variableDeclarations.length; i++) {
				res.variableDeclarations[i] = variableDeclarations[i].deepCopy();
			}
		}

		res.variableDeclarationsOriginal = new VariableDeclaration[variableDeclarationsOriginal.length];
		for(int i = 0; i < res.variableDeclarationsOriginal.length; i++) {
			res.variableDeclarationsOriginal[i] = variableDeclarationsOriginal[i].deepCopy();
		}

		res.procedureDeclarations = new ProcedureDeclaration[procedureDeclarations.length];
		for(int i = 0; i < res.procedureDeclarations.length; i++) {
			res.procedureDeclarations[i] = procedureDeclarations[i].deepCopy();
		}

		res.instructions = new Instruction[instructions.length];
		for(int i = 0; i < res.instructions.length; i++) {
			res.instructions[i] = instructions[i].deepCopy();
		}

		return res;
	}
}
